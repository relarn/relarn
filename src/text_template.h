// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Functions to generate messages from templates and the game's
// internal state.

#ifndef HDR_TEXT_TEMPLATE_H
#define HDR_TEXT_TEMPLATE_H

#include "player.h"

char *text_expand(char *template, const struct Player *pl);

#endif
