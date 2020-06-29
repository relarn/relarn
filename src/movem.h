// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module handles monster movement including decisions on how
// they are to act (i.e. the "AI").

#ifndef HDR_GUARD_MOVEM_H
#define HDR_GUARD_MOVEM_H

/* movem.c */
void movemonst(void);
void teleportmonst(int xx, int yy);
void lasthit(uint8_t x, uint8_t y);

#endif
