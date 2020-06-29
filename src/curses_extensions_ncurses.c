// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Helper functions and definitions to build against ncurses (as
// opposed to PDCurses).  We use ncurses when building console-mode
// (i.e. tty) versions of ReLarn on Unixish systems.
//
// We put stuff here to minimize the number of preprocessor conditions
// (i.e. #ifdef's) we in the code.

#include <stdlib.h>
#include <curses.h>

// Sanity check; ensure we're using ncurses
#ifndef NCURSES_VERSION
#   error "This file should only be compiled when using ncurses."
#endif

void
setup_curses_preconditions() {
}

// In ncurses, we use white and black respectively for showing
// unrevealed squares on a light or dark screen.
void
setup_unseen_area_colors(short lightpair, short darkpair) {
    init_pair(lightpair, -1, COLOR_WHITE);
    init_pair(darkpair, -1, COLOR_BLACK);
}// setup_unseen_area_colors

void
setup_curses_extensions() {
    // We default to a much shorter ESC delay than the default because
    // we're assuming the user is not on a serial terminal and ESC is
    // used a lot.
    if (!getenv("ESCDELAY")) {
        set_escdelay(50);       // 50 ms
    }
}

void
show_notification_msg(const char *msg) {
    printf("%s\n", msg);
}

// ncurses is always tty-based
bool is_tty() { return true; }
