// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module provides the global options. These are set from either
// the config file, the command line or taken from the OS.

#ifndef HDR_GUARD_SETTINGS_H
#define HDR_GUARD_SETTINGS_H

#include "player.h"
#include "constants.h"

struct Options {
    int difficulty;
    
    enum GENDER gender;
    bool genderSet;

    enum GENDER spouseGender;
    bool spouseGenderSet;
    
    char name[PLAYERNAME_MAX];
    bool nameSet;
    
    enum CHAR_CLASS cclass; // Char. class; uses CCNONE to mean unset
    
    bool nointro;
    bool nonap;
    bool nobeep;
    char emailClient[MAX_CMDLINE_LENGTH];
    
    bool showFoV;           // Highlight area of visibility
    bool showUnrevealed;    // Show unexplored sections as gray
    bool drawDebugging;     // Debug option
};

extern struct Options GameSettings;

void initopts(void);
void readopts(const char *);

#endif
