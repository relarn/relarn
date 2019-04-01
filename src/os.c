// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Support for Unix-style OSs.

#include "os.h"

#include "internal_assert.h"
#include "map.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>


// This needs to be defined on the compiler command line
#ifndef INST_ROOT
#   error "INST_ROOT is undefined."
#endif


#define LIB_SUBPATH "/share/relarn/lib/"
#define VAR_SUBPATH "/var/relarn/"


//
// Names of various game files
//
#define SCORENAME       "Relarn-scoreboard"
#define HELPNAME        "Uhelp"
#define INTRONAME       "Uintro"
#define LEVELSNAME      "Umaps"
#define MAILFILE        "Ujunkmail"
#define FORTSNAME       "Ufortune"


static void setup_signals(void);


static bool exeIsWritable = false;


static void
make_game_dir_if_needed() {
    char cfgpath[MAXPATHLEN];
    snprintf(cfgpath, sizeof(cfgpath), "%s/" BASE_CFGDIR, home());
    ENSURE_MSG(strlen(cfgpath) < sizeof(cfgpath)-1, "$HOME path is too long.");

    struct stat sb;
    int statted = !stat(cfgpath, &sb);
    if (statted && S_ISDIR(sb.st_mode)) { return; }

    ENSURE_MSG(statted == 0, "File '" BASE_CFGDIR "' is not a directory.");

    int mdstat = mkdir(cfgpath, 0755);
    if (mdstat) {
        printf("Error creating '%s': %s", cfgpath, strerror(errno));
        exit(1);
    }// if

    printf("Created '%s'\n", cfgpath);    // DEBUG?
}// make_game_dir_if_needed


void
init_os(int argc, char *argv[]) {

    // Create the ~/.relarn directory if not present
    make_game_dir_if_needed();
    
    // Set canDebug; true if the executable is writable by this user.
    ASSERT(argc > 0);
    char *binpath = argv[0];
    exeIsWritable = !access(binpath, W_OK);

    setup_signals();
}

// Return a default player name using the current user's ID
const char *
get_username() {
    static char name[PLAYERNAME_MAX];
    if (name[0]) { return name; }
    
    struct passwd *pwe = getpwuid(geteuid());
    if (!pwe) {
        fprintf(stderr, "Unable to determine user-ID.\n");
        exit(1);
    }/* if */

    strncpy(name, pwe->pw_name, sizeof(name) - 1);
    name[sizeof(name)-1] = 0;

    return name;
}/* init_os*/


/* Return whether debugging (i.e "wizard mode") is allowed.  On Unixy
 * platforms, this is true if the executable is writable by the
 * current user. */
bool
canDebug() {
    return exeIsWritable;    
}// canDebug


const char*
home() {
    static char home[MAXPATHLEN];
    if (home[0]) { return home; }

    char *env_home = getenv("HOME");
    ENSURE_MSG(env_home && *env_home, "$HOME is unset.");

    strncpy(home, env_home, sizeof(home));
    home[sizeof(home) - 1] = 0;
    ENSURE_MSG(strlen(home) < sizeof(home) - 1, "$HOME path is too long.");
    
    return home;
}// home


// Determine the installation root; this is either from the
// environment or falls back onto a default.
static const char *
inst_root() {
    static char *root;

    if (!root) {
        const char *env_root = getenv(VAR_PATH);
        root = (env_root && *env_root) ? xstrdup(env_root) : INST_ROOT;
    }// if 
    
    return root;
}// inst_root



static const char *
make_path(char *dest, size_t dest_size, const char *root, const char *subpath,
          const char *file) {
    if (dest[0]) { return dest; }

    snprintf(dest, dest_size, "%s/%s/%s", root, subpath, file);
    ENSURE_MSG(strlen(dest) < dest_size - 1, "Expected filepath is too long!");

    return dest;
}// make_path

static const char *
make_cfgdir_path(char *dest, size_t dest_size, const char *file) {
    return make_path(dest, dest_size, home(), BASE_CFGDIR, file);
}// make_cfgdir_path



const char *
fortunes_path() {
    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), inst_root(), LIB_SUBPATH, FORTSNAME);
}// fortunes_path

const char *
junkmail_path() {
    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), inst_root(), LIB_SUBPATH, MAILFILE);
}// junkmail_path

const char *
levels_path() {
    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), inst_root(), LIB_SUBPATH, LEVELSNAME);
}// levels_path

const char *
intro_path() {
    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), inst_root(), LIB_SUBPATH, INTRONAME);
}// intro_path

const char *
help_path() {
    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), inst_root(), LIB_SUBPATH, HELPNAME);
}// help_path

const char *
scoreboard_path() {
    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), inst_root(), VAR_SUBPATH, SCORENAME);
}// scoreboard_path





const char *
savefile_path() {
    static char path[MAXPATHLEN];
    return make_cfgdir_path(path, sizeof(path), BASE_SAVE);
}// savefile_path

static const char *
backup_savefile_path() {
    static char newname[MAXPATHLEN+3];

    strncpy(newname, savefile_path(), sizeof(newname));

    size_t len = strlen(newname);
    newname[len - 4] = '\0';    // Truncate the trailing extension
    strcat(newname, ".1.sav");

    return newname;
}

const char *
cfgfile_path() {
    static char path[MAXPATHLEN];
    return make_cfgdir_path(path, sizeof(path), BASE_CFG);
}// cfgfile_path

const char *
mailfile_path() {
    static char path[MAXPATHLEN];
    return make_cfgdir_path(path, sizeof(path), BASE_MAILBOX);
}// opsfile_path


// Return the user-id, caching locally to save a bit of overhead.
long
get_user_id() {
    static bool uid_set = false;
    static long uid;

    if (uid_set) { return uid; }
    
    uid = (long)geteuid();
    uid_set = true;

    return uid;
}// get_user_id

static bool
rotate_save() {
    const char *sp = savefile_path();

    // If no save file is present, there's nothing to do.
    if (access(sp, F_OK) != 0) { return true; }

    return rename(sp, backup_savefile_path()) == 0;
}// rotate_save


enum SAVE_STATUS
save_game() {
    const char *sp = savefile_path();

    bool moveSuccess = rotate_save();

    FILE *fh = fopen(sp, "wb");
    if (!fh) { return SS_FAILED; }
    
    int status = savegame_to_file(fh);

    fclose(fh);
    
    if (!status) {
        return SS_FAILED;
    }

    return moveSuccess ? SS_SUCCESS : SS_RENAME_FAILED;
}// save_game



static enum SAVE_STATUS
load_savefile(const char *sp) {
    FILE *fh = fopen(sp, "rb");
    if (!fh) { return SS_NO_SAVED_GAME; }
    
    bool wrongVersion = false;
    bool status = restore_from_file(fh, &wrongVersion);
    fclose(fh);
    
    if (wrongVersion) {
        return SS_INCOMPATIBLE_SAVE;
    }

    return status ? SS_SUCCESS : SS_FAILED;
}// load_savefile


enum SAVE_STATUS
restore_game() {
    const char *path = savefile_path();
    enum SAVE_STATUS status = load_savefile(path);

    // If the savefile failed, look for a backup and delete the
    // defective save on successful backup load
    if (status == SS_NO_SAVED_GAME || status == SS_FAILED) {
        status = load_savefile(backup_savefile_path());
        if (ss_success(status)) {
            unlink(path);
            status = SS_USED_BACKUP;
        }
    } else {
        // Otherwise, rotate the save file to backup
        rotate_save();
    }// if 

    return status;
}// restore_game

void
delete_save_files() {
    unlink(backup_savefile_path());
    unlink(savefile_path());
}// delete_save_files



// Maintain a flag indicating that it's safe to autosave. This should
// only be true while waiting for user input.
static bool
access_safety_flag(bool set, bool val) {
    volatile static bool safe_to_save = false;
    if (set) {
        ASSERT(safe_to_save != val);
        safe_to_save = val;
    }// if 

    return safe_to_save;
}

// Enable/disable/test the emergency autosave flag. The caller must
// ensure that this is true only when the game state is coherent
// (e.g. while waiting for the player's input at the start of a turn).
void enable_emergency_save() { access_safety_flag(true, true); }
void disable_emergency_save() { access_safety_flag(true, false); }
bool safe_to_emergency_save() { return access_safety_flag(false, false); }
    

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

    // Save the current game if that's enabled.
    bool savedGame = false;
    if (safe_to_emergency_save()) {
        enum SAVE_STATUS ss = save_game();
        savedGame = ss_success(ss);
    }

    // Restore stdout to a normal console
    teardown_ui();

    // Report the error to the user
    printf("Caught signal %d; game %s saved!\n", sig,
           savedGame ? "successfully" : "NOT");

    // And quit
    exit(1);
}// exit_by_signal



static void
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


// Takes an advisory lock on 'path'.  Does not return until the lock
// is owned.  Returns an int that should be 
LOCK_HANDLE
lock_file(const char *path) {
    int fh = open(path, O_RDWR);
    if (fh < 0) { return fh; }

    int status = lockf(fh, F_LOCK, 0);
    if (status != 0) {
        close(fh);
        return -1;
    }

    return fh;
}// lock_file


void
unlock_file(LOCK_HANDLE fh) {
    if (fh < 0) { return; } // Tolerate invalid/closed handles.

    int stat = lockf(fh, F_ULOCK, 0);
    if (stat) {
        // We can't do more than issue a warning at this point.
        perror("Error unlocking file:");
    }// if 
}// unlock_scorefile


// Produce and return a suitable seed value for the randomizer based
// on operating-system properties.
unsigned long
get_random_seed() {
    unsigned long seed = (unsigned long)time(NULL);
    seed ^= (unsigned long)getpid();

    return seed;
}// get_random_seed
