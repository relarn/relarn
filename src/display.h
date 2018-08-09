// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


// High-level (i.e. entity-level) control of the display.

#ifndef HDR_GUARD_DISPLAY_H
#define HDR_GUARD_DISPLAY_H

#include "map.h"

void update_stats(void);
void showcellat(int x, int y, int xradius, int yradius);
void showplayerarea(void);
void show1cell(int x, int y);
void update_display(bool thorough);

#endif
