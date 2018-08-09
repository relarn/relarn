// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


// Data struct to hold a collection of lines of text for viewing by
// the UI and related code to manipulate it.  Actual display is in
// showpages() in ui.[ch]. 
//
// Text between pipes ('|') are displayed standout and text between
// underscores ('_') are displayed underlined.  '/' pauses for 1
// second (after forcing a refresh to display what came before) for
// the dramatic pause.  Note that this behaviour is implemented by
// showpages().
//
// Like PickList, TextBuffer follows the OOP pattern (but with the
// rendering method in ui.c).  Thus, struct fields should be
// considered private outside of this module.

#ifndef HDR_GUARD_TEXTBUFFER_H
#define HDR_GUARD_TEXTBUFFER_H

#include <stdbool.h>

#define INF_BUFFER (-1)

struct TextBuffer {
    char **text;        /* Text lines, NULL-terminated array of strings */
    int num_lines;      /* Number of lines in 'text'. */
    bool newline;       /* Next append starts a new line. */

    int max_lines;      /* Max. lines in 'text' or -1 for unlimited. */
    int max_width;      /* Max. num. chars in a line in 'text'. */
};

struct TextBuffer *tb_malloc(int length, int linewidth);
void tb_free(struct TextBuffer *tb);
void tb_appendline(struct TextBuffer *tb, const char *line);
void tb_append(struct TextBuffer *tb, const char *line);
const char *tb_getlastn(struct TextBuffer *tb, int index, int windowSize);
void tb_center_all(struct TextBuffer *tb);


static inline int tb_num_lines(struct TextBuffer *tb) { return tb->num_lines; }

#endif
