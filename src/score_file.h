// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Module for accessing the scoreboard file.

#ifndef HDR_GUARD_SCORES_H
#define HDR_GUARD_SCORES_H

#include "player.h"
#include "textbuffer.h"

void ensureboard(void);
void showscores(bool all);
bool write_scores(const char *filename);
long get_taxes_owed(void);
bool newscore(long score, bool won, int level, const char *ending,
              const struct Player *uu);


#endif
