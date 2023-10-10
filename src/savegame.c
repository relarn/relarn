// This file is part of ReLarn; Copyright (C) 1986 - 2023; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "savegame.h"

#include "version_info.h"
#include "player.h"
#include "map.h"
#include "store.h"

// PLATFORM_ID is an ID specific to the OS+CPU so we can detect
// incompatible save files.  It needs to get set on the command line.
// Currently uses the result of `uname -psr` but can be anything
// provided that it's specific to that architecture and OS.
#ifndef PLATFORM_ID
#   error "PLATFORM_ID is undefined."
#endif

// WARNING: this must be the same length as the generated string!
#define RELARN_SAVE_ID_FMT "ReLarn " VERSION " " PLATFORM_ID " %d\n"

struct SaveGame {
    char header[sizeof(RELARN_SAVE_ID_FMT)];
    struct Player uu;
    struct World world;
    struct Object invent[IVENSIZE];
    struct StoreItem shopInvent[OBJ_COUNT];
    unsigned shopInventSz;
};

static struct SaveGame CurrentSave;
volatile static bool stashedGameExists = false;
volatile static bool stashOperationInProgress = false;


// Test to see if a stashed game exists.  This is used for sanity
// checks.
bool stashed_game_present() { return stashedGameExists; }
bool stash_op_in_progress() { return stashOperationInProgress; }


// Compute the "magic string" that begins a save file and is checked
// against to confirm that this is a valid savefile from a compatible
// ReLarn version.  (This used to be a macro but it turns out it's
// actually pretty difficult to distinguish between 32-bit and 64-bit
// targets at compile time.)
static const char*
savefile_magic_string() {
    static char magic_string[sizeof(RELARN_SAVE_ID_FMT)];
    
    if (!magic_string[0]) {
        snprintf(magic_string, sizeof(magic_string),
                 RELARN_SAVE_ID_FMT, (int)(8*sizeof(void*)));
    }

    return magic_string;
}// savefile_magic_string


// Copy the game state from the global state (and 'world') into the
// SaveGame at CurrentSave.
void
stash_game_state() {

    // We set stashOperationInProgress because we may try to do an
    // emergency save from a signal handler and that could be invoked
    // in the middle of a stash operation.  This lets us detect that
    // case.
    ASSERT(!stashOperationInProgress);
    stashOperationInProgress = true;

    strcpy(CurrentSave.header, savefile_magic_string());

    CurrentSave.uu = UU;

    stash_global_world_at(&CurrentSave.world);

    memcpy(CurrentSave.invent, &Invent, sizeof(Invent));

    memcpy(CurrentSave.shopInvent, &ShopInvent, sizeof(ShopInvent));
    CurrentSave.shopInventSz = ShopInventSz;

    stashedGameExists = true;
    stashOperationInProgress = false;
}// stash_game_state



void
restore_global_game_state() {
    ASSERT(stashedGameExists);

    UU = CurrentSave.uu;

    restore_global_world_from(&CurrentSave.world);

    memcpy(&Invent, CurrentSave.invent, sizeof(Invent));

    memcpy(&ShopInvent, CurrentSave.shopInvent, sizeof(ShopInvent));
    ShopInventSz = CurrentSave.shopInventSz;
}// restore_global_game_state




/* Compute a checksum for 'data'. */
static unsigned int
sum(unsigned char *data, size_t data_len) {
    unsigned int sum = 0;
    for (size_t nb = 0; nb < data_len; nb++) {
        int c = *data++;

        if (sum & 0x1) {
            sum = (sum >> 1) + 0x8000;
        } else {
            sum >>= 1;
        }

        sum += c;
        sum &= 0xFFFF;
    }

    return sum;
}/* sum*/


bool
save_stashed_game_to_file(FILE *fh) {
    ASSERT(stashedGameExists);

    if (stashOperationInProgress) {
        return false;
    }// if

    unsigned char *buf = (unsigned char*)&CurrentSave;
    unsigned int checksum = sum(buf, sizeof(struct SaveGame));

    size_t written = fwrite(buf, 1, sizeof(struct SaveGame), fh);
    written       += fwrite(&checksum, 1, sizeof(checksum), fh);

    return written == sizeof(checksum) + sizeof(struct SaveGame);
}// save_stashed_game_to_file


bool
load_stashed_game_from_file(FILE *fh, bool *wrongFileVersion) {

    ASSERT(!stashOperationInProgress);  // Whoah!

    struct SaveGame game;
    size_t read = fread(&game, 1, sizeof(struct SaveGame), fh);

    if (!streq(savefile_magic_string(), game.header)) { return false; }

    unsigned int filesum = 0;
    read += fread(&filesum, 1, sizeof(filesum), fh);

    if (read != sizeof(struct SaveGame) + sizeof(filesum)) { return false; }

    unsigned int computed_filesum = sum((unsigned char *)&game, sizeof(game));
    if (filesum != computed_filesum) { return false; }

    CurrentSave = game;
    stashedGameExists = true;

    return true;
}// load_stashed_game_from_file
