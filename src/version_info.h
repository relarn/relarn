// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This header contains the release version number.  It also provides
// fallbacks for BUILD_DATE and COMMIT_ID so that the code will
// compile if those symbols are not defined by the build tool.

#ifndef HDR_VERSION_INFO_H
#define HDR_VERSION_INFO_H

// Version and patchlevel are stored in version.mk and set by the
// makefile.
#if !defined(RELARN_VERSION) || !defined(RELARN_PATCHLEVEL)
#   error Version symbols not set.
#endif

// We only use these symbols so that code which doesn't import this
// header (and the check above) will not compile.
#define VERSION    RELARN_VERSION
#define PATCHLEVEL RELARN_PATCHLEVEL



//
// These may or may not be set by the build process; if not, the
// compile should continue.
//

// Build timestamp; this is a freeform string
#ifndef BUILD_DATE
#   define BUILD_DATE ""
#endif

// The git commit ID of this release (if built from git and git works
// on the build machine.)
#ifndef COMMIT_ID
#   define COMMIT_ID ""
#endif

// Return a string containing the version, build date and commit ID
// used to produce this executable.
inline static const char *version_str() {
    static char buffer[120];
    const char *bd = BUILD_DATE, *cid = COMMIT_ID;
    snprintf(buffer, sizeof(buffer), "%s.%s%s%s%s%s",
             VERSION, PATCHLEVEL,
             bd ? " " : "", bd,
             cid ? " " : "", cid);
    return buffer;
}

#endif
