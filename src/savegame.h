// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Code and data structures for holding a saved game and writing it to
// disk.  This is separate from the actual game state so that we can
// save at any time and not worry about the game state being
// inconsistent.  We do this by copying the game state to memory at
// the end of each turn and then saving that as needed.

#ifndef HDR_SAVEGAME_H
#define HDR_SAVEGAME_H

#include <stdbool.h>
#include <stdio.h>

void stash_game_state(void);
void restore_global_game_state(void);
bool save_stashed_game_to_file(FILE *fh);
bool load_stashed_game_from_file(FILE *fh, bool *wrongFileVersion);
bool stashed_game_present(void);
bool stash_op_in_progress(void);

#endif
