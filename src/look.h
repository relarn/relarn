// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Code to implement the behaviour of an object when the player steps
// into the square it occupies.

#ifndef HDR_GUARD_LOOK_H
#define HDR_GUARD_LOOK_H

#include <stdbool.h>

void cancel_look(void);
void force_look (void);
bool lookforobject(void);

#endif
