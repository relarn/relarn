// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Os-specific functionality.  All os-dependent stuff is buried here.

#include "os.h"

#include "internal_assert.h"
#include "map.h"
#include "ui.h"
#include "player.h"
#include "savegame.h"
#include "game.h"

#ifdef __WIN32__
#   include "os_windows.h"
#else
#   include "os_unix.h"
#endif

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>

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
#define SAMPLERC        "relarnrc.sample"
#define ICONNAME        "relarn-icon.bmp"

//
// Names of player-side config files
//
#define BASE_SAVEDIR    "savegame"
#define BASE_SAVE       "relarn.sav"
#define BASE_CFG        "relarnrc"
#define BASE_MAILBOX    "inbox"

static bool exeIsWritable = false;


static const char *samplerc_path(void);


// Copy a file from src to dest.
static bool
copy_file(const char *src, const char *dest) {
    FILE *srcfh = fopen(src, "rb");
    if (!srcfh) { return false; }

    FILE *destfh = fopen(dest, "wb");
    if (!destfh) {
        fclose(srcfh);
        return false;
    }

    bool status = true;
    for (int c = getc(srcfh); c != EOF; c = getc(srcfh)) {
        if (fputc(c, destfh) == EOF) {
            status = false;
            break;
        }// if 
    }// for 

    fclose(srcfh);
    fclose(destfh);
        
    return status;
}// copy_file


// Return the full path to the game's main directory.  On the first
// call, check if the path exists and attempt to create the directory
// if not.  It is a fatal error if this can't be done.
static const char *
cfgdir() {
    static char cfgpath[MAXPATHLEN];
    if (cfgpath[0]) { return cfgpath; }

    // Compute the path and store it in cfgpath
    snprintf(cfgpath, sizeof(cfgpath), "%s/" BASE_CFGDIR, cfg_root());
    ENSURE_MSG(strlen(cfgpath) < sizeof(cfgpath)-1,
               "config path is too long.");

    // If the path now exists, we can return it.
    struct stat sb;
    int statted = !stat(cfgpath, &sb);
    if (statted && S_ISDIR(sb.st_mode)) { return cfgpath; }

    // Otherwise, we try to create the directory.
    ENSURE_MSG(statted == 0, "File '" BASE_CFGDIR "' is not a directory.");

    int mdstat = os_mkdir(cfgpath, 0755);
    if (mdstat) {
        notify("Error creating '%s': %s", cfgpath, strerror(errno));
        exit(1);
    }// if

    say("Created '%s'\n", cfgpath);

    // Attempt to install an initial config file.
    if( !copy_file(samplerc_path(), cfgfile_path()) ) {
        say("Error installing sample '%s'.\n", BASE_CFG);
        say("You can find it in '%s' if you need it.\n", samplerc_path());
    }// if
    
    return cfgpath;
}// cfgdir


void
init_os(const char *binpath) {

    // Create the ~/.relarn directory if not present
    cfgdir();

    // Set canDebug; true if the executable is writable by this user.
    exeIsWritable = !access(binpath, W_OK);

    setup_signals();
}

// Return a default player name using the player's user name as set in
// the environment.
const char *
get_username() {
    static char name[PLAYERNAME_MAX];
    if (name[0]) { return name; }

    const char *env_name = getenv(ENV_USER);
    if (!env_name || !env_name[0]) {
        env_name = "Mysterio";
    }

    strncpy(name, env_name, sizeof(name) - 1);
    name[sizeof(name) - 1] = 0;

    // Filter out any tricky characters (esp. tab)
    for (char* c = name; *c; c++) {
        if (!isgraph(*c)) {
            *c = ' ';
        }// if
    }// for

    return name;
}

/* Return whether debugging is allowed.  On Unixy platforms, this is
 * true if the executable is writable by the current user; for
 * Windows, we use an environment variable.  */
bool
canDebug() {
    return exeIsWritable || os_win_debug();
}// canDebug


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

    // If the file part was empty, we remove the trailing slash
    if (!*file) {
        dest[strlen(dest) - 1] = '\0';
    }// if

    return dest;
}// make_path

static const char *
make_cfgdir_path(char *dest, size_t dest_size, const char *file) {
    return make_path(dest, dest_size, cfg_root(), BASE_CFGDIR, file);
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
icon_path() {
    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), inst_root(), LIB_SUBPATH, ICONNAME);
}// help_path

static const char *
samplerc_path() {
    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), inst_root(), LIB_SUBPATH, SAMPLERC);
}// help_path


const char *
scoreboard_path() {
    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), inst_root(), VAR_SUBPATH, SCORENAME);
}// scoreboard_path



const char *
cfgdir_path() {
    static char path[MAXPATHLEN];
    return make_cfgdir_path(path, sizeof(path), "");
}// cfgdir_path


static void
make_savedir_if_absent() {
    static bool check_done = false;

    // Only check once per execution
    if (check_done) { return; }
    check_done = true;

    // We assume the main .relarn dir has already been created.
    char path[MAXPATHLEN];
    snprintf(path, sizeof(path), "%s/%s", cfgdir(), BASE_SAVEDIR);

    // If the savedir already exists, we're done.
    struct stat sb;
    int statted = !stat(path, &sb);
    if (statted && S_ISDIR(sb.st_mode)) { return; }

    // Otherwise, we try to create the directory, quitting on error.
    int mdstat = os_mkdir(path, 0755);
    if (mdstat) {
        notify("Error creating '%s': %s", path, strerror(errno));
        exit(1);
    }// if
}// make_savedir_if_absent



// Get path to savefile.  Also creates savedir if not present.
static const char *
savefile_path() {
    make_savedir_if_absent();

    static char path[MAXPATHLEN];
    return make_path(path, sizeof(path), cfg_root(),
                     BASE_CFGDIR "/" BASE_SAVEDIR, BASE_SAVE);
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
const char *
get_user_id() {
    static char uid[OS_UID_STR_MAX];

    if (!uid[0]) {
        os_get_user_id(uid, sizeof(uid));
    }// if

    return uid;
}// get_user_id

// Makes the current save the (latest?) backup.
bool
rotate_save() {
    const char *sp = savefile_path();
    const char *bp = backup_savefile_path();
    
    // If no save file is present, there's nothing to do.
    if (access(sp, F_OK) != 0) { return true; }

    // Windows doesn't let you rename a file onto an existing file so
    // we unlink it first.
    os_unlink(bp);

    return rename(sp, bp) == 0;
}// rotate_save


enum SAVE_STATUS
save_game() {
    const char *sp = savefile_path();

    // This should never happen, but we test for it here in case
    // something has gone dramatically wrong.
    if (!stashed_game_present()) {
        notify("Savegame buffer is empty; not saving!");
        return SS_FAILED;
    }
    
    bool moveSuccess = rotate_save();

    FILE *fh = fopen(sp, "wb");
    if (!fh) { return SS_FAILED; }

    int status = save_stashed_game_to_file(fh);

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
    bool status = load_stashed_game_from_file(fh, &wrongVersion);
    fclose(fh);

    // This is overkill but the conditional means that a failed load
    // won't overwrite the current game.
    if (status == SS_SUCCESS) {
        restore_global_game_state();
    }
    
    if (wrongVersion) {
        return SS_INCOMPATIBLE_SAVE;
    }

    return status ? SS_SUCCESS : SS_FAILED;
}// load_savefile


enum SAVE_STATUS
restore_game() {
    const char *path = savefile_path();

    // Load the game
    enum SAVE_STATUS status = load_savefile(path);

    // If the savefile failed, look for a backup and delete the
    // defective save on successful backup load
    if (status == SS_NO_SAVED_GAME || status == SS_FAILED) {
        status = load_savefile(backup_savefile_path());
        if (ss_success(status)) {
            unlink(path);
            status = SS_USED_BACKUP;
        }
    }// if

    // And finish the setup.
    if (ss_success(status)) {
        post_restore_processing();
    }
    
    return status;
}// restore_game

void
delete_save_files() {
    unlink(backup_savefile_path());
    unlink(savefile_path());
}// delete_save_files





// Locks the file at 'fh'.  This must be called IMMEDIATELY after
// opening the file (i.e. before any other file operations have been
// performed).  It will block until a lock has been obtained.
//
// It returns true on success and false on failure, although that's
// not necessarily really useful.  We mostly assume that it's going to
// work and that the consequences of failure are pretty small.
bool
lock_file(FILE *fh) {
    int fnum = fileno(fh);
    int status = os_lockf(fnum);

    if (status != 0) {
        notify("Error obtaining file lock: %s\n", strerror(errno));
        return false;
    }

    return true;
}// lock_file


// Unlock a file previously locked iwth lock_file().
void
unlock_file(FILE *fh) {
    int fnum = fileno(fh);
    int status = os_unlockf(fnum);
    if (status != 0) {
        // We can't do more than issue a warning at this point.
        notify("Error unlocking file: %s", strerror(errno));
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
