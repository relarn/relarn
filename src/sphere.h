// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Code and data for maintaining & moving the spheres of
// annihilation.

#ifndef HDR_GUARD_SPHERE_H
#define HDR_GUARD_SPHERE_H

struct sphere
{
    struct sphere *p;   /* pointer to next structure */
    short x,y,lev;      /* location of the sphere */
    short dir;      /* direction sphere is going in */
    short lifetime;     /* duration of the sphere */
};
struct sphere *spheres;


void newsphere(int x, int y, int dir, int life);
void rmsphere(int x, int y);
void sphboom(int x, int y);
void movsphere(void);

#endif
