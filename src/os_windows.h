// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Windows-specific OS stuff.

#ifndef HDR_OS_WIN32_H
#define HDR_OS_WIN32_H

//#include <windows.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/locking.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// We define our own versions of these
#undef min
#undef max

// stat (the function and struct) seem compatible except for the name.
#define stat _stat

// Env. var holding the username
#define ENV_USER "USERNAME"

// Name of the config directory
#define BASE_CFGDIR "ReLarn"

// Env. Var. holding path to the home dir.
#define OVERRIDE_HOME "RELARN_CONFIG_HOME"


void os_get_user_id(char *uid, size_t uid_size);
int os_mkdir(const char *path, unsigned mode);
int os_lockf(int fnum);
int os_unlockf(int fnum);
int os_setenv(const char *name, const char *value, int overwrite);
int os_unsetenv(const char *name);
const char* cfg_root(void);


// Signals aren't really a thing on Windows
static inline void setup_signals() { /* stub */ }
static inline int os_unlink(const char *filename) { return _unlink(filename); }

// The usual test (writable executable) is tricky on Windows so we
// also allow an environment variable as an option.
static inline bool os_win_debug(void) { return !!getenv("I_CHEAT_AT_RELARN"); }

#endif
