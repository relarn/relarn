// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Type and helpers for representing character gender.

#ifndef HDR_GUARD_GENDER_H
#define HDR_GUARD_GENDER_H

enum GENDER {FEMALE, MALE, NONBINARY};
#define GENDER_STRINGS {"female","male","nonbinary"}  // Order matches the enum

static inline const char *woman(enum GENDER gender) {
    static const char *pronoun[] = {"woman", "man", "person"};
    return pronoun[gender];
}// woman

static inline const char *wife(enum GENDER gender) {
    static const char *pronoun[] = {"wife", "husband", "spouse"};
    return pronoun[gender];
}// wife

static inline const char *female(enum GENDER gender) {
    static const char *pronoun[] = {"female", "male", "non-binary"};
    return pronoun[gender];
}// female


#endif
