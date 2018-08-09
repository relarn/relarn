// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "internal_assert.h"

#include "textbuffer.h"

static void splitAndAppend(struct TextBuffer *tb, const char *line);
static void scrollBuffer (struct TextBuffer *tb);
static void appendSegment (struct TextBuffer *tb, char *line);
static char shortenLine (int width, char *ptr);
static char *mk_lastline(struct TextBuffer *tb, const char *line);
static char *dropLast(struct TextBuffer *tb);




/* Create and initialize a TextBuffer */
struct TextBuffer *
tb_malloc(int length, int linewidth) {
    struct TextBuffer *result;

    result = xmalloc (sizeof(struct TextBuffer));

    result->text = NULL;
    result->num_lines = 0;
    result->newline = true;
    result->max_lines = length;
    result->max_width = linewidth;

    return result; 
}/* tb_malloc*/


/* Delete a TextBuffer and anything it points to. */
void
tb_free(struct TextBuffer *tb) {
    int n;

    if (!tb) {
        return;
    }/* if */

    for (n = 0; n < tb->num_lines; n++) {
        free(tb->text[n]);
    }/* for */
    
    free (tb->text);

    free(tb);
}/* tb_free*/


/* Append text to the end of the buffer, scrolling up if needed.
 * Splits on newlines.  'line' only ends the line if it ends with a
 * newline.  */
void
tb_append(struct TextBuffer *tb, const char *line) {
    if (!*line) return;
        
    splitAndAppend (tb, line);
    scrollBuffer(tb);
}/* tb_append*/

/* Like tb_append() but also ends the last line. */
void
tb_appendline(struct TextBuffer *tb, const char *line) {
    int ll;

    if (!*line) {
        line = "\n";
    }/* if */

    tb_append(tb, line);

    ll = strlen(line);
    if (line[ll - 1] != '\n') {
        tb_append(tb, "\n");
    }/* if */
}/* tb_appendline*/


/* Return a pointer to a string in the end of the buffer.  The string
 * should not be modified in any way.  This is used to show the last
 * windowSize lines in a scrolling window.  Currently no way to scroll
 * back--FIX. */
const char *
tb_getlastn(struct TextBuffer *tb, int index, int windowSize) {
    ASSERT(index < windowSize);

    if (windowSize < tb->num_lines) {
        return tb->text[index + (tb->num_lines - windowSize)];
    }/* if */

    if (index > tb->num_lines - 1) {
        return "";
    }/* if */

    return tb->text[index];
}/* tb_getlastn*/


/* Append 'line' to 'tb', splitting it into multiple lines if it is
 * too long for a single line. */
static void
splitAndAppend(struct TextBuffer *tb, const char *line) {
    char *linedup;
    char *ptr;
    char savedChar;
    int maxlen;

    ASSERT(*line);

    maxlen = tb->max_width;

    linedup = mk_lastline(tb, line);
    ptr = linedup;
    while (1) {
        /* Null-terminate the end of the line at ptr. */
        savedChar = shortenLine(maxlen, ptr);

        /* Append the current line (ptr to the null) to the buffer. */
        appendSegment(tb, ptr);

        /* Advance ptr to the start of the following line. */
        ptr += strlen (ptr) + 1;

        /* If it's empty, we've hit the extra null added above and
         * we're done. */
        if (!*ptr) {
            break;
        }/* if */

        /* If it turns out that the character overwritten by the null
         * added by shortenLine() was important (i.e. not a space), we
         * need to restore it. */
        if (!isspace(savedChar)) {
            --ptr;
            *ptr = savedChar;
        }/* if */
    }/* while */

    free (linedup);
}/* splitAndAppend*/


/* Make a working copy of 'line' with an extra null at the end.  If
 * this is to be appended to the last line, first pull off that line
 * and prepend it to the copy.  Also, set tb->newline to true if line
 * ends with a newline.*/
static char *
mk_lastline(struct TextBuffer *tb, const char *line) {
    int llen;
    char *linedup, *lastline;
    bool setNewline = false;

    llen = strlen(line);
    setNewline = line[llen - 1] == '\n';

    if (!tb->newline) {
        lastline = dropLast(tb);
        llen += strlen(lastline);
    }/* if */

    linedup = xmalloc (llen + 2);
    linedup[0] = 0;

    if (!tb->newline) {
        strcpy(linedup, lastline);
        free(lastline);
    }/* if */

    strcat (linedup, line);
    linedup[llen + 1] = 0;  /* Extra trailing null. */

    /* Store the EOL-ness of the current line in tb->newline. */
    tb->newline = setNewline;

    return linedup;
}/* mk_lastline*/



/* Insert a null into the string at 'ptr' to make it shorter than
 * 'width' and return the character that was overwritten.  Tries to
 * split on whitespacebut. */
static char
shortenLine (int width, char *ptr) {
    int end, plen;
    char replaced;

    plen = strlen(ptr);

    /* Walk forward until we either exceed 'width' or find a
     * newline. */
    for (end = 0; end < width; end++) {
        if (!ptr[end] || ptr[end] == '\n') {
            break;
        }/* if */
    }/* for */

    /* If ptr is shorter than width and there are no newlines, we're
     * done. */
    if (end == plen) {
        return 0;
    }/* if */

    /* Walk back from 'end' to find the last whitespace character.  If
     * found, this is where the line gets broken. */
    for (; end >= 0; end--) {

        if (isspace(ptr[end])) {
            replaced = ptr[end];
            ptr[end] = 0;
            return replaced;
        }/* if */

    }/* for */

    /* If we get here, it means the string segment doesn't have a
     * blank, so we just insert the null at the last character. */
    replaced = ptr[width - 1];
    ptr[width - 1] = 0;

    return replaced;
}/* shortenLine */


/* Append 'line' to the list of lines. */
static void
appendSegment (struct TextBuffer *tb, char *line) {
    int n;
    char *linecopy;

    /* Normalize all whitespace, except for the \f used to mark a page
     * end. */
    for (n = 0; line[n]; n++) {
        if (isspace(line[n]) && (line[n] != '\f' && n != 0)) {
            line[n] = ' ';
        }/* if */
    }/* for */
        
    
    linecopy = xstrdup(line);

    ++tb->num_lines;
    tb->text = xrealloc(tb->text, tb->num_lines * sizeof(char **));

    tb->text[tb->num_lines - 1] = linecopy;
}/* appendSegment */

/* Unreference the last line from 'tb' and return a pointer to it. */
static char *
dropLast(struct TextBuffer *tb) {
    char *last;

    if (tb->num_lines <= 0) return NULL;

    last = tb->text[tb->num_lines - 1];
    --tb->num_lines;

    /* We don't resize tb->text here; appendSegment can do that. */

    return last;
}/* dropLast*/


/* Remove lines from the start of the buffer until there are no more
 * than tb->max_lines lines in it. */
static void
scrollBuffer (struct TextBuffer *tb) {
    int scrollAmt, n;

    scrollAmt = tb->num_lines - tb->max_lines;

    if (tb->max_lines < 0 || scrollAmt <= 0) {
        return;
    }/* if */

    for (n = 0; n < scrollAmt; n++) {
        free (tb->text[n]);
    }/* for */

    for (n = scrollAmt; n < tb->num_lines; n++) {
        tb->text[n - scrollAmt] = tb->text[n];
    }/* for */

    /* Adjust num_lines and null-terminate tb->text */
    tb->num_lines -= scrollAmt;
    tb->text[tb->num_lines] = NULL;
}/* truncateBuffer */

// Reposition all lines so that they're centered.
void
tb_center_all(struct TextBuffer *tb) {
    for (int n = 0; n < tb->num_lines; n++) {
        size_t len = strlen(tb->text[n]);
        
        if (len >= tb->max_width - 1) { continue; }

        char *line = xmalloc(tb->max_width + 1);
        snprintf(line, tb->max_width + 1, "%*s%s",
                 (tb->max_width - (int)len)/2, "",
                 tb->text[n]);
        free(tb->text[n]);
        tb->text[n] = line;
    }// for 
}// tb_center_all

