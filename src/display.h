// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


// High-level (i.e. entity-level) control of the display.  Also owns
// the field-of-view.
//
// This module maintains dirty flags to keep track of what needs to be
// redrawn.  Calling see_and_update_*(), etc. marks the relevant
// section dirty and update_display() (re)draws the changes if
// necessary.
//
// (Everything here is map-centric but calling update_display() also
// updates the other windows; this is a small failure in abstraction.)


#ifndef HDR_GUARD_DISPLAY_H
#define HDR_GUARD_DISPLAY_H

#include "map.h"

void update_display(void);      // Draw if something's changed

void redraw(void);              // Redraw everything

// See & mark dirty everything in FoV
void see_and_update_fov(void);

// The player sees x,y and the cell will be redrawn next update;
// bypasses FoV
void see_and_update_at(int x, int y);

// Test if x,y is within the player's view
bool player_sees(int x, int y);

// Ensure that entire map is redrawn next update
void force_full_update();

// Display `c` colored as an effect at x,y for `period` milliseconds
void flash_at(int x, int y, char c, int period);

#endif
