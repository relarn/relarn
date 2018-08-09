// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module provides the global options. These are set from either
// the config file, the command line or taken from the OS.

#ifndef HDR_GUARD_SETTINGS_H
#define HDR_GUARD_SETTINGS_H

#include "player.h"
#include "constants.h"

struct Options {
    int difficulty;
    enum SEX sex;
    bool sexSet;
    char name[PLAYERNAME_MAX];
    bool nointro;
    bool nonap;
    enum CHAR_CLASS cclass;
    bool nobeep;
    char emailClient[MAX_CMDLINE_LENGTH];
};

extern struct Options GameSettings;

void initopts(void);
void readopts(const char *);

#endif
