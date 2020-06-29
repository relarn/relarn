// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Unix-specific stuff.

#ifndef HDR_OS_UNIX_H
#define HDR_OS_UNIX_H

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//#include <pwd.h>

// Env. var holding the username
#define ENV_USER "USER"

// Name of the config directory
#define BASE_CFGDIR ".relarn"


void setup_signals(void);
const char* cfg_root(void);

static inline int os_mkdir(const char *path, mode_t mode) {
    return mkdir(path, mode);
}

static inline void os_get_user_id(char *uid, size_t uid_size) {
    snprintf(uid, uid_size, "%ld", (long)geteuid());
}

// Wrappers around file locking.
//
// Note: Should only be called just after opening and just before
// closing respectively.
static inline int os_lockf(int fnum) { return lockf(fnum, F_LOCK, 0); }
static inline int os_unlockf(int fnum) { return lockf(fnum, F_ULOCK, 0); }
static inline int os_unlink(const char *filename) { return unlink(filename); }
static inline bool os_win_debug(void) { return false; }

#endif
