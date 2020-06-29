// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Code and data for maintaining & moving the spheres of
// annihilation.

#ifndef HDR_GUARD_SPHERE_H
#define HDR_GUARD_SPHERE_H

#include "util.h"

#include <stdint.h>

#define MAX_SPHERES 20

struct SphereState {
    int8_t x, y, lev;      /* location of the sphere */
    DIRECTION dir;         /* direction sphere is going in */
    uint8_t lifetime;      /* duration of the sphere; zero means none */
};

struct SphereList {
    struct SphereState spheres[MAX_SPHERES];
};


void newsphere(struct SphereList* spheres, int x, int y, DIRECTION dir);
void updatespheres(struct SphereList *spheres);
void rmsphere(struct SphereList *spheres, int x, int y);

#endif
