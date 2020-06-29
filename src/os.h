// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module implements the OS-specific code.  In theory, a port to
// a new OS could replace this os.c entirely while preserving the
// interface but there are probably better ways of doing this (that
// aren't C preprocessor abuse, that is.)

#ifndef HDR_GUARD_OS_H
#define HDR_GUARD_OS_H

#include <stdbool.h>
#include <stdio.h>

#define OS_UID_STR_MAX 128

enum SAVE_STATUS {
    SS_FAILED,              // Error doing the thing
    SS_SUCCESS,             // Success doing the thing
    SS_USED_BACKUP,         // Main file failed but backup worked
    SS_RENAME_FAILED,       // Saved but unable to backup previous savefile
    SS_NO_SAVED_GAME,       // No saved game found
    SS_INCOMPATIBLE_SAVE,   // Saved game is from a different release
};

typedef int LOCK_HANDLE;

// Per-user paths
//const char *home(void);
const char *cfgdir_path(void);
const char *cfgfile_path(void);
const char *mailfile_path(void);
const char *get_username(void);

// Resource file paths
const char *fortunes_path(void);
const char *junkmail_path(void);
const char *levels_path(void);
const char *intro_path(void);
const char *help_path(void);
const char *icon_path(void);
const char *scoreboard_path(void);


void init_os(const char *binpath);
bool canDebug(void);
const char * get_user_id(void);

enum SAVE_STATUS save_game(void);
enum SAVE_STATUS restore_game(void);
bool rotate_save(void);
void delete_save_files(void);

bool lock_file(FILE *fh);
void unlock_file(FILE *fh);

unsigned long get_random_seed(void);

int os_setenv(const char *name, const char *value, int overwrite);
int os_unsetenv(const char *name);


static inline bool ss_success(enum SAVE_STATUS ss) {
    return ss == SS_SUCCESS || ss == SS_RENAME_FAILED || ss == SS_USED_BACKUP;
}// ss_success


#ifdef __WIN32__

// Win32 changed the names of random and srandom, 'cuz standards are
// for suckers, apparently.  These are defined in os_windows.c but we
// need them here so that util.h can see them.
long random(void);
void srandom(unsigned seed);

#endif


#endif
