/* Copyright 2016 Chris Reuter. */

#ifndef HDR_STRINGBUILDER_H
#define HDR_STRINGBUILDER_H

// Class-pattern for an object that constructs a string by appending
// or removing characters from the end.
//
// Length may not exceed LONG_MAX.


#include <stdlib.h>


struct StringBuilder {
    // Private:
    char *buffer;
    size_t length;
};


struct StringBuilder *sb_alloc(void);
void sb_free(struct StringBuilder *sb);
void sb_append(struct StringBuilder *sb, const char *text);
void sb_append_char(struct StringBuilder *sb, char c);
char *sb_str_and_free(struct StringBuilder *sb);
void sb_reset(struct StringBuilder *sb);
void sb_drop(struct StringBuilder *sb, size_t count);
char sb_at(struct StringBuilder *sb, long n);

inline static size_t sb_len(struct StringBuilder *sb) { return sb->length; }
inline static const char *sb_str(struct StringBuilder *sb) { return sb->buffer;}

#endif
