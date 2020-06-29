// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "os.h"

#include "util.h"

// The fortune to use if the load fails. This is probably overkill
static const char *FALLBACK[] = {
    "Help! I'm being held prisoner in a fortune cookie factory!",
};

static const char **flines = NULL;  // array of pointers to each fortune
static size_t nlines = 0;           // length of flines



static void
loadfortunes() {
    FILE *fh;
    char line[255], *status;

    fh = fopen(fortunes_path(), "r");
    if (!fh) return;

    while(true) {
        size_t last;

        status = fgets(line, sizeof(line), fh);
        if (!status) break;

        last = strlen(line) - 1;
        if (line[last] == '\n') {
            line[last] = '\0';
        }/* if */

        ++nlines;
        flines = xrealloc(flines, nlines * sizeof(char *));
        flines[nlines - 1] = xstrdup(line);
    }/* while*/

    fclose(fh);
}/* loadfortunes*/


// Returns a fortune.  The first call loads them from the fortunes
// file or, failing that, uses the fallback.
const char *
fortune() {
    if (!nlines) {
        loadfortunes();

        if (!nlines) { /* i.e. loadfortunes() failed. */
            nlines = 1;
            flines = FALLBACK;
        }/* if */
    }/* if */
        
    return flines[rund(nlines)];
}/* fortune*/


