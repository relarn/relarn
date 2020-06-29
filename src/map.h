// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module contains code and data structures to manage the
// individual maps.
//
// Note that unlike most of the game state, the maps and level
// information are private to the file map.c.  (This is how it all
// should be but we're not there yet.)

#ifndef HDR_GUARD_MAP_H
#define HDR_GUARD_MAP_H

#include "constants.h"
#include "monster.h"
#include "object.h"
#include "internal_assert.h"

#include <stdio.h>

#define MAX_STOLEN  32

#define MAXLEVEL 16     /*  max # levels in the dungeon + 1 */
#define MAXVLEVEL 5     /*  max # of levels in the volcano */

#define NLEVELS (MAXLEVEL + MAXVLEVEL)
#define DBOTTOM (MAXLEVEL - 1)
#define VTOP    (DBOTTOM + 1)
#define VBOTTOM (MAXLEVEL + MAXVLEVEL - 1)

#define MAXX 67
#define MAXY 17

#define CAVE_EXIT_X 33
#define CAVE_EXIT_Y (MAXY - 1)

struct MapSquare {
    struct Monster mon;
    struct Object obj;
    struct Object recalled;
};

struct Level {
    struct MapSquare map[MAXX][MAXY];
    bool exists;
    bool known;
    struct Object stolen[MAX_STOLEN];
    unsigned numStolen;
};

// The game world state.  (Note: the main instance is private to
// map.c)
struct World {
    int levelNum;                       // Current level (-1 for none)
    struct Level levels[NLEVELS];       // The levels
};

struct MapSquare* at(uint8_t x, uint8_t y);
struct Level *lev(void);

void see_at(int x, int y);
void forget_at(int x, int y);
bool known(struct MapSquare here);
bool known_at(int x, int y);

int getlevel(void);
const char *getlevelname(void);
void setlevel(int newlevel, bool identify);
bool savegame_to_file(FILE *fh);
bool restore_from_file(FILE *fh, bool *wrongFileVersion);
void init_cells(void);
void add_to_stolen(struct Object thing);
struct Object remove_stolen(struct Level *lev);
bool findobj(uint8_t type, int8_t *x, int8_t *y);
void remake_map_keeping_contents(void);
void createitem(int x, int y, struct Object item);
void create_rnd_item(int x, int y, int lev);
void creategem(void);
bool cgood(int x, int y, bool itm, bool monst);
void set_reveal(bool see);
void udelobj(void);
void heal_monsters(void);
void get_level_copy_at(int index, struct Level *lev);

void restore_global_world_from(const struct World *aWholeNewWorld);
void stash_global_world_at(struct World *worldCopy);


// Restrict x, y to the inside of the map
static inline void clip(int8_t *x, int8_t *y) {
    *x = *x < 0     ? 0         :
        *x >= MAXX  ? MAXX - 1  :
        *x;

    *y = *y < 0     ? 0         :
        *y >= MAXY  ? MAXY - 1  :
        *y;
}

static inline bool inbounds(int x, int y) {
    return x >= 0 && x < MAXX && y >= 0 && y < MAXY;
}

#endif
