// This file is part of ReLarn; Copyright (C) 1986 - 2023; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "os_unix.h"

#include "os.h"
#include "ui.h"
#include "constants.h"
#include "internal_assert.h"

#include <signal.h>
#include <unistd.h>

//
// Signal Stuff
//

// ULarn used to catch everything and do a bunch of stuff with
// it. I've gotten rid of most of those because the defaults already
// generally do the right thing.
//
// The only case where we catch now is if it's possible to save the
// game before quitting.

// Note: Signals are part of ANSI-C but have very minimal defined
// behaviour. We are assuming something sufficiently Unix-like here.
//
// Unix does signals differently across platforms, which is also a
// source of endless amusement, but we bypass most of that here by
// quitting after getting a signal so the differences don't really
// matter.
//
// If you wish to do anything more clever, you need to be much more
// careful.  You are probably better off using sigaction(); it has
// much better-defined semantics.


// Handler for graceful-exit signals
static void
exit_by_signal(int sig) {

    // Disable all signals so that nothing interrupts the cleanup
    sigset_t mask;
    sigemptyset(&mask);
    sigprocmask(SIG_SETMASK, &mask, NULL);

    // Restore stdout to a normal console
    teardown_ui();

    // Report the error to the user
    printf("Caught signal %d!  Quitting!\n", sig);

    // And quit
    exit(1);
}// exit_by_signal


void
setup_signals() {
    // Polite exit; typically sent via 'kill'
    signal(SIGTERM, exit_by_signal);

    // Polite exits via terminal keystrokes, which are disabled by
    // ncurses. These are here just in case.
    signal(SIGINT, exit_by_signal);
    signal(SIGQUIT, exit_by_signal);

    // Exit via terminal disconnection (or console window close).
    signal(SIGHUP, exit_by_signal);
}// setup_signals


int
os_setenv(const char *name, const char *value, int overwrite){
    return setenv(name, value, overwrite);
}

int
os_unsetenv(const char *name) {
    return unsetenv(name);
}


// Parent directory for the config file.  On Unix, this is just the
// home directory.
const char*
cfg_root() {
    static char home[MAXPATHLEN];
    if (home[0]) { return home; }

    char *env_home = getenv("HOME");
    ENSURE_MSG(env_home && *env_home, "$HOME is unset.");

    zstrncpy(home, env_home, sizeof(home));
    home[sizeof(home) - 1] = 0;
    ENSURE_MSG(strlen(home) < sizeof(home) - 1, "$HOME path is too long.");

    return home;
}// cfg_root

