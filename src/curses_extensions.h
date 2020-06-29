// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Abstraction layer around non-standard extensions provided by the
// two (currently) implementations of curses we can use.
//
// This should ONLY be included by ui.c.
//
// We put stuff here to minimize the number of preprocessor conditions
// (i.e. #ifdef's) we in the code.

#ifndef HDR_GUARD_CURSES_EXTENSIONS_H
#define HDR_GUARD_CURSES_EXTENSIONS_H

void setup_curses_preconditions(void);
void setup_curses_extensions(void);
void setup_unseen_area_colors(short lightpair, short darkpair);

void show_notification_msg(const char *msg);
//bool is_tty(void);    // Moved to ui.h

#endif
