// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module contains the general(ish) purpose utility code.

#ifndef HDR_GUARD_UTIL_H
#define HDR_GUARD_UTIL_H

#include "os.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>



/* Symbolic names for directions of movement.  Must be kept in synch
   with adjpoint(). */
typedef enum {
    DIR_CANCEL    = 0,  /* This means no action performed. */
    DIR_STAY      = 1,  /* This means action is 'stay here'. */
    DIR_NORTH     = 2,
    DIR_EAST      = 3,
    DIR_SOUTH     = 4,
    DIR_WEST      = 5,
    DIR_NORTHEAST = 6,
    DIR_NORTHWEST = 7,
    DIR_SOUTHEAST = 8,
    DIR_SOUTHWEST = 9,

    DIR_MIN_DIR   = DIR_NORTH,      // First dir. that's a real direction
    DIR_MAX       = DIR_SOUTHWEST,  // Last dir. that's a real direction
} DIRECTION;



bool freadable(const char *filename);
void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
char *xstrdup(const char *);
void *xrealloc(void *ptr, size_t size);
char **splitstring(const char* orig, int* nitems);
void adjpoint(int8_t x, int8_t y, DIRECTION dir, int8_t *outx, int8_t *outy);

const char *an(const char *word);

static inline int rnd(int x)  { return (random() % x) + 1; }
static inline int rund(int x) { return random() % x; }
static inline int min(int x, int y) { return x > y ? y : x; }
static inline int max(int x, int y) { return x < y ? y : x; }
static inline long min_l(long x, long y) { return x > y ? y : x; }
static inline long max_l(long x, long y) { return x < y ? y : x; }

static inline int clamp(int amt, int minv, int maxv) {
    return min(max(amt, minv), maxv);
}

static inline DIRECTION randdir() {
    return rund(DIR_MAX - DIR_MIN_DIR) + DIR_MIN_DIR;
}

// Normalize all whitespace characters to space so they can be used as
// delimiters
static inline void ws_to_space(char *txt) {
    int n;
    for (n = 0; txt[n]; n++) {
        if (isspace(txt[n])) { txt[n] = ' '; }
    }
}


// Null-tolerant string equality
static inline bool streq(const char *s1, const char *s2) {
    if (s1 == s2) { return true; }
    if (s1 == NULL || s2 == NULL) { return false; }
    return strcmp(s1, s2) == 0;
}


#endif


