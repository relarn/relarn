// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Type and helpers for representing character gender.

#ifndef HDR_GUARD_GENDER_H
#define HDR_GUARD_GENDER_H

enum GENDER {FEMALE, MALE, NONBINARY};
#define GENDER_STRINGS {"female","male","nonbinary"}  // Order matches the enum

static inline const char *woman(enum GENDER gender) {
    static const char *term[] = {"woman", "man", "person"};
    return term[gender];
}// woman

static inline const char *wife(enum GENDER gender) {
    static const char *term[] = {"wife", "husband", "spouse"};
    return term[gender];
}// wife

static inline const char *female(enum GENDER gender) {
    static const char *term[] = {"female", "male", "non-binary"};
    return term[gender];
}// female

static inline const char *pretty(enum GENDER gender) {
    static const char *term[] = {"pretty", "handsome", "attractive"};
    return term[gender];
}// female


#endif
