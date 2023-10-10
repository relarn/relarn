// This file is part of ReLarn; Copyright (C) 1986 - 2023; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "util.h"

#include "internal_assert.h"


/* Test if 'filename' is a file that exists and is readable. */
bool
freadable(const char *filename) {
    FILE *fh;

    fh = fopen (filename, "r");
    if (!fh) {
        return false;
    }/* if */

    fclose (fh);
    return true;
}/* freadable*/


/* Call malloc() with 'size' and die if there's an error. */
void *
xmalloc(size_t size) {
    void *result;

    result = malloc(size);
    ENSURE_MSG(!!result, "malloc() failed.");

    return result;
}// xmalloc


/* Call calloc() with 'size' and die if there's an error. */
void *
xcalloc(size_t count, size_t size) {
    void *result;

    result = calloc(count, size);
    ENSURE_MSG(!!result, "calloc() failed.");

    return result;
}// xcalloc



/* Just like realloc (and calls it, in fact) but dies with an error if
 * the call returns NULL (i.e. fails).  Don't call it with arguments
 * that would legitimately trigger a return value of NULL (e.g. size
 * == 0). */
void *
xrealloc(void *ptr, size_t size) {
    void *result;

    result = realloc(ptr, size);
    ENSURE_MSG(!!result, "realloc() failed.");

    return result;
}// xrealloc


/* Call malloc() with 'size' and die if there's an error. */
char *
xstrdup(const char *src) {
    char *result;

    result = strdup(src);
    ENSURE_MSG(!!result, "strdup() failed.");

    return result;
}// xstrdup

/* Return the appropriate indefinite article ("a" or "an") to precede
 * 'word'. */
const char *
an(const char *word) {
    if (strchr("aeiou", tolower(*word))) return "an";
    return "a";
}/* an*/


/* Given a string 'orig', split it along newlines ('\n') and return an
 * array of strings containing each segment.  Sets *nitems to the
 * number of items in the array.  Both the array and its contents must
 * be freed, but the string is allocated in one contiguous block so the
 * caller should only ever free the first element.  Memory errors are
 * instantly fatal.*/
char **
splitstring(const char* orig, int* nitems) {
    char *curr, *copy;
    char **splitstr = NULL;
    int segments;

    copy = xstrdup(orig);

    segments = 1;
    splitstr = xmalloc(segments * sizeof(char *));
    splitstr[0] = copy;

    for (curr = copy; *curr; curr++) {
        if (*curr != '\n') continue;

        *curr = 0;
        ++segments;
        splitstr = xrealloc(splitstr, segments * sizeof(char *));
        splitstr[segments - 1] = curr + 1;
    }

    *nitems = segments;
    return splitstr;
}/* splitstr*/



// Given a point (x, y) and direction 'dir', set (*outx, *outy) to the
// location adjacent to (x,y) that would result from a single step in
// that direction.
//
// Non-direction values of dir (DIR_CANCEL and DIR_STAY) result in no
// movement (i.e. (*outx, *outy) == (x, y)).
void
adjpoint(int8_t x, int8_t y, DIRECTION dir, int8_t *outx, int8_t *outy) {
    static const int offx[] = { 0, 0,  0, 1, 0, -1,  1, -1, 1, -1 };
    static const int offy[] = { 0, 0, -1, 0, 1,  0, -1, -1, 1,  1 };

    ASSERT(dir < sizeof(offx)/sizeof(offx[0]) &&
           dir < sizeof(offy)/sizeof(offy[0]));
    
    *outx = x + offx[dir];
    *outy = y + offy[dir];
}// adjpoint


// Wraps strncpy but ensures that dest is always null-terminated,
// overwriting the last copied character if necessary.
char* zstrncpy(char *dest, const char *src, size_t max) {
    ASSERT(max > 0);
    char* result = strncpy(dest, src, max);
    dest[max - 1] = 0;
    return result;
}// zstrncpy
