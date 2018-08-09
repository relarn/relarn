
#include "stringbuilder.h"

#include <assert.h>
#include <limits.h>

#include "util.h"


// Create a new StringBuilder
struct StringBuilder*
sb_alloc() {
    struct StringBuilder *sb = xcalloc(1, sizeof(struct StringBuilder));
    sb->buffer = xcalloc(1, sizeof(char));
    return sb;
}

// Free all memory associated with 'sb'
void
sb_free(struct StringBuilder *sb) {
    if (!sb) { return; }    // Tolerate NULL argument

    free(sb->buffer);
    free(sb);
}

// Append the contents of 'text' to sb.
void
sb_append(struct StringBuilder *sb, const char *text) {
    if (! *text) { return; }

    size_t len = strlen(text);
    sb->length += len;
    sb->buffer = xrealloc(sb->buffer, sb->length + 1);
    strcat(sb->buffer, text);

    assert(sb->length == strlen(sb->buffer));
    assert(sb->length < LONG_MAX);
}

// Append a single char to 'sb'.
void
sb_append_char(struct StringBuilder *sb, char c) {
    // TODO: make this more efficient
    char cb[2];
    cb[0] = c;
    cb[1] = 0;
    sb_append(sb, cb);
}

// Free the builder, returning the alloc'd content.
char *
sb_str_and_free(struct StringBuilder *sb) {
    assert(sb);
    
    const char *str = sb->buffer;
    free(sb);
    return (char *)str;
}// sb_free_str

void
sb_reset(struct StringBuilder *sb) {
    sb->buffer = xrealloc(sb->buffer, 1);
    sb->buffer[0] = '\0';
    sb->length = 0;
}// sb_reset


// Delete up to 'count' characters from the end of the string.  If
// count > the length of the string, it is restricted to the string
// length.
void
sb_drop(struct StringBuilder *sb, size_t count) {
    if (count <= sb->length) {
        sb->length -= count;
    } else {
        sb->length = 0;
    }// if .. else

    sb->buffer = xrealloc(sb->buffer, sb->length + 1);
    sb->buffer[sb->length] = 0;
}// sb_drop

// Return the character at position n or 0 if invalid. Negative
// indexes return relative to the end (i.e. -1 is the last character).
char
sb_at(struct StringBuilder *sb, long n) {
    if (n < 0) {
        n += (int)sb->length;
    }// if 

    if (n < 0 || n >= sb->length) { return 0; }
    return sb->buffer[n];
}// sb_at
