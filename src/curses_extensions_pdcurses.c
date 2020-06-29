// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Helper functions and definitions to build against PDCurses+SDL2
// (i.e. PDCurses compiled against SDL 2 so that it manages its own
// window instead of using a terminal.)
//
// We put stuff here to minimize the number of preprocessor conditions
// (i.e. #ifdef's) we in the code.

#include "curses_extensions.h"

#include "os.h"

#include "version_info.h"
#include "constants.h"
#include "settings.h"
#include "ui.h"

#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <sdl2/pdcsdl.h>


// Sanity check
#if !USE_PDCURSES
#   error "This file should only be compiled when using PDCurses."
#endif


// PDCurses takes a bunch of parameters from the environment so that's
// how we need to set it.  (*sigh*).
static void
set_pdcurses_font() {

    // Remove existing PDC settings
    os_unsetenv("PDC_FONT");
    os_unsetenv("PDC_FONT_SIZE");

    // Set the font
    if (GameSettings.fontPath[0]) {
        char *fp = GameSettings.fontPath;

        // Expand leading '+' to the config directory.
        char path[MAXPATHLEN];
        if (GameSettings.fontPath[0] == '+') {
            snprintf(path, sizeof(path), "%s/%s",
                     cfgdir_path(),
                     GameSettings.fontPath + 1);
            fp = path;
        }// if

        // Confirm we can read the file.  We do this here instead of
        // letting PDCurses try and fail because the error message
        // gets lost on Windows.
        if (!freadable(fp)) {
            notify("Unable to open font file '%s'.\n", fp);
            exit(1);
        }// if 
        
        os_setenv("PDC_FONT", fp, 1);

        // Set the font size
        char sizebuf[80];
        snprintf(sizebuf, sizeof(sizebuf), "%d", GameSettings.fontSize);
        os_setenv("PDC_FONT_SIZE", sizebuf, 1);
    }// if
}// set_pdcurses_font


static void
clr(struct Color *clr, uint8_t r, uint8_t g, uint8_t b) {
    if (clr->isSet) { return; }

    *clr = (struct Color) {true, r, g, b};

    /* clr->r = r; */
    /* clr->g = g; */
    /* clr->b = b; */
    /* clr->isSet = true; */
}// clr



// Colours to use by default if not set by the user.  Also set
// darkScreen if the relevant colors haven't been set.
static void
set_pdcurses_default_colors() {

    // On SDL, we default to dark screen.
    if (!GameSettings.darkScreenSet) {
        GameSettings.darkScreen = true;
    }

    clr(&GameSettings.black, 0x22, 0x22, 0x22);
    clr(&GameSettings.red, 0xFF, 0x77, 0x77);
    clr(&GameSettings.green, 0x67, 0xF8, 0x6A);
    clr(&GameSettings.yellow, 0xFF, 0xFF, 0x66);
    clr(&GameSettings.blue, 0x00, 0x00, 0xFF);
    clr(&GameSettings.magenta, 0x44, 0x44, 0x44);
    clr(&GameSettings.cyan, 0x00, 0xFF, 0xFF);
    clr(&GameSettings.white, 0xAA, 0xAA, 0xAA);
}// set_pdcurses_default_colors


static void
set_pdcurses_icon() {
    os_setenv("PDC_ICON", icon_path(), 1);
}// set_pdcurses_icon



void
setup_curses_preconditions() {
    set_pdcurses_font();
    set_pdcurses_default_colors();
    set_pdcurses_icon();
}// setup_curses_preconditions


void
setup_curses_extensions() {
    PDC_set_title("ReLarn " VERSION "." PATCHLEVEL);

    // No Resizing Allowed!
    SDL_SetWindowResizable(pdc_window, false);
}


// In PDCurses+SDL, we use magenta (which is changed to something
// appropriate) for undiscovered cells.
void
setup_unseen_area_colors(short lightpair, short darkpair) {
    init_pair(lightpair, -1, COLOR_MAGENTA);
    init_pair(darkpair, -1, COLOR_MAGENTA);
}// setup_unseen_area_colors


void
show_notification_msg(const char *msg) {
    int status = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                                          "Hey!", msg, NULL);

    // Last-ditch fallback.  Probably not necessary.
    if (status != 0) {
        printf("%s\n", msg);
    }
}// show_notification_msg

// PDCurses is always GUI-based
bool is_tty() { return false; }
