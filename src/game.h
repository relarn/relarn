// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Module implementing the main game loop.

#ifndef HDR_GUARD_GAME_H
#define HDR_GUARD_GAME_H

#include "monster.h"
#include "cast.h"

/* Death codes to pass to die. */
enum GAME_ENDING {
    DD_DUMMY = NUM_MONSTERS + 1,   // Ensure count doesn't overlap a creature

#define ENDING(id, name) id,
#   include "game_endings.h"
#undef ENDING
};

struct GameState {
    unsigned monstCount;    // Countdown 'til next random monster creation
    bool wizardMode;        // Enables certain debug features.
    bool spellknow[SPNUM];  // This is the array of spells currently known.
};

extern struct GameState GS;


bool onemove(DIRECTION dir, bool noMonMove);
void mainloop(void);
void makeplayer();
void cancelLook(void);
void cancelMonMove(void);
void graceful_exit(const char *msg);
long compute_score(bool won);
void save_and_quit(void);
long compute_taxes_owed(const struct Player *);
void game_over_probably(unsigned cause);


#endif
