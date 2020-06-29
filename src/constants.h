// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This file holds most of the global constants

#ifndef HDR_CONSTANTS_H
#define HDR_CONSTANTS_H


// Space to allocate for holding a file path.  Since there's
// (probably) no *actual* OS limit, we just use a sufficiently big
// number and be done with it.
#ifndef MAXPATHLEN
#  define MAXPATHLEN 1024
#endif

// Maximum length of a command-line to run.  This is kind of
// arbritrary, but should be long enough for anyone.
#define MAX_CMDLINE_LENGTH 256


//
// Various maxima
//

#define MAXPLEVEL 100   /* maximum player level allowed     */
#define SCORESIZE 25    /* max number of people on a scoreboard max */
#define TIMELIMIT 40000 /* maximum number moves before the game is called*/
#define MOBUL 100       /* Number of turns in a mobul */
#define TAXRATE (0.05)  /* the tax rate for the LRS = 5%  */


//
// Environment variables
//
#define VAR_PATH        "RELARN_INSTALL_ROOT"


#define COPYRIGHT                                                       \
  "ReLarn Copyright (C) 1986-2020 by The Authors.\n"                    \
  "ReLarn comes with ABSOLUTELY NO WARRANTY.\n"                         \
  "You may redistribute copies of ReLarn\n"                             \
  "under the terms of the GNU General Public License.\n"                \
  "For more information about these matters, see the\n"                 \
  "file named LICENSE.txt."

#endif
