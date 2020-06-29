// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Module implementing the main game loop.

#ifndef HDR_GUARD_GAME_H
#define HDR_GUARD_GAME_H

#include "monster.h"
#include "cast.h"
#include "util.h"
#include "player.h"

/* Death codes to pass to die. */
enum GAME_ENDING {
    DD_DUMMY = LAST_MONSTER + 1,   // Ensure count doesn't overlap a creature

#define ENDING(id, name) id,
#   include "game_endings.h"
#undef ENDING
};


bool onemove(DIRECTION dir);
void mainloop(void);
void makeplayer();
void cancelMonMove(void);
void graceful_exit(const char *msg);
long compute_score(bool won);
void save_and_quit(void);
long compute_taxes_owed(const struct Player *);
void game_over_probably(unsigned cause);
void post_restore_processing(void);
void emergency_save(void);
void cancel_emergency_save(void);

#endif
