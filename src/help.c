// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "help.h"

#include "ui.h"
#include "os.h"



/* Load a formatted text file and return its contents as an array of
 * pointers. */
static struct TextBuffer *
load_doc (const char *filename) {
    FILE *fh;
    char line[82];
    struct TextBuffer *tb;

    tb = tb_malloc (INF_BUFFER, sizeof(line));

    fh = fopen (filename, "r");
    if (!fh) {
        return NULL;
    }/* if */

    while (fgets(line, sizeof(line), fh)) {
        tb_appendline(tb, line);
    }/* while */

    return tb;
}/* load_doc */

/* Display the contents of 'filename' in a pager. */
static void
show_doc (const char *filename) {
    struct TextBuffer *helptext;

    helptext = load_doc(filename);
    if (!helptext) {
        return;
    }/* if */

    showpages(helptext);

    tb_free(helptext);
}/* help */

// Display the online help
void
help() {
    show_doc (help_path());
}

/*
 *  function to display the welcome message and background
 */
void
welcome () {
    show_doc (intro_path());
}/* welcome */

