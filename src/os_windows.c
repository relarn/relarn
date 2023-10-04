
#include "os_windows.h"
#include "constants.h"

#include "internal_assert.h"

#include <stdlib.h>

#define SECURITY_WIN32
#include <windows.h>
#include <secext.h>




void os_get_user_id(char *uid, size_t uid_size) {
    ULONG sz = uid_size;

    // Getting the SID for the user is an astounding chore, so we
    // don't bother.  Instead, we just use the username, specifically
    // the sAMAccountName which is universally supported.

    bool stat = GetUserNameExA(NameSamCompatible, uid, &sz);
    ENSURE_MSG(stat, "Unable to get the username.");
}// os_get_user_id


// The windows way of doing mkdir:
int
os_mkdir(const char *path, unsigned mode) {
    return _mkdir(path);
    (void)mode;             // suppress compiler warning
}



// On Windows, locking is sort of vaguely similar to the *nix version,
// but attempting to lock will eventually time out if the file remains
// locked by another process.  This shouldn't be an issue for us since
// we don't keep files open for very long.
//
// Note: Should only be called just after opening and just before
// closing respectively.

#define LOCK_FILE_RANGE 0x7FFFFFFF  // 0 doesn't work on Windows so we
                                    // do this to lock the entire
                                    // file.

int
os_lockf(int fnum) {
    // We assume that fnum is at the start of the file.
    return _locking(fnum, _LK_LOCK, LOCK_FILE_RANGE);
}

int
os_unlockf(int fnum) {
    // Locking happens at the current file position, so we need to
    // move to the start.
    long lseek_ret = _lseek(fnum, 0L, SEEK_SET);
    if (lseek_ret != 0) { return lseek_ret; }

    return _locking(fnum, _LK_UNLCK, LOCK_FILE_RANGE);
}


// Wrappers around rand() and srand().
//
// Note: rand() returns an int instead of a long so it's possible that
// the results will be biased downward if sizeof(int) < sizeof(long).
// However, this is not a problem the way we use the result.
long random() { return (long)rand(); }
void srandom(unsigned seed) { srand(seed); }

#undef LOCK_FILE_RANGE


int
os_setenv(const char *name, const char *value, int overwrite) {
    char buffer[512];

    ASSERT(overwrite);

    snprintf(buffer, sizeof(buffer), "%s=%s", name, value);
    return _putenv(buffer);
}// setenv


// Shim to (mostly) mimic unsetenv.
int
os_unsetenv(const char *name) {
    return os_setenv(name, "", 1);
}// unsetenv

// Get the Documents folder (or equivalent) from the environment.
const char*
cfg_root() {
    static char root[MAXPATHLEN];
    if (root[0]) { return root; }

    char *env_home = getenv(OVERRIDE_HOME);
    if (env_home) {
        zstrncpy(root, env_home, sizeof(root));
        root[sizeof(root) - 1] = 0;
        return root;
    }

    env_home = getenv("HOMEPATH");
    char *homedrive = getenv("HOMEDRIVE");
    ENSURE_MSG(env_home && *env_home && homedrive && *homedrive,
               "Unable to read HOMEPATH and HOMEDRIVE or " OVERRIDE_HOME);
    snprintf(root, sizeof(root), "%s%s/Documents", homedrive, env_home);
    ENSURE_MSG(strlen(root) < sizeof(root) - 1, "$HOME path is too long.");
    
    return root;
}
