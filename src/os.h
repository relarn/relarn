// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module implements the OS-specific code.  In theory, a port to
// a new OS could replace this os.c entirely while preserving the
// interface but there are probably better ways of doing this (that
// aren't C preprocessor abuse, that is.)

#ifndef HDR_GUARD_OS_H
#define HDR_GUARD_OS_H

#include <stdbool.h>


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
const char *home(void);
const char *savefile_path(void);
const char *cfgfile_path(void);
const char *mailfile_path(void);
const char *get_username(void);

// Resource file paths
const char *fortunes_path(void);
const char *junkmail_path(void);
const char *levels_path(void);
const char *intro_path(void);
const char *help_path(void);
const char *scoreboard_path(void);


void init_os(int argc, char *argv[]);
bool canDebug(void);
long get_user_id(void);
void enable_emergency_save(void);
void disable_emergency_save(void);
bool safe_to_emergency_save(void);

enum SAVE_STATUS save_game(void);
enum SAVE_STATUS restore_game(void);
void delete_save_files(void);

LOCK_HANDLE lock_file(const char *path);
void unlock_file(LOCK_HANDLE fh);

unsigned long get_random_seed(void);


static inline bool ss_success(enum SAVE_STATUS ss) {
    return ss == SS_SUCCESS || ss == SS_RENAME_FAILED || ss == SS_USED_BACKUP;
}// ss_success

static inline bool lock_success(LOCK_HANDLE lh) { return lh >= 0; }

#endif
