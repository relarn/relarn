// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module manages spell casting.

#ifndef HDR_GUARD_CAST_H
#define HDR_GUARD_CAST_H

struct SpellInfo {
    char *code;
    char *name;
    char *desc;
};
extern const struct SpellInfo Spells[];

/* Big enum of spells. */
enum SPELL {
#define SPELL(id, code, name, ldesc) id,
#include "spell_list.h"
#undef SPELL
    SPNUM
};


void cast(void);
void godirect(enum SPELL spnum, int dam, char *str, int delay, char cshow);

enum PICKMODE {
    PM_CAST,    /* Picked for casting. */
    PM_LEARN,   /* Pick for learning via genie. */
};
int pickspell (enum PICKMODE pm);



#endif
