// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "display.h"
#include "ui.h"
#include "game.h"

#include "sphere.h"



// Special behaviours when a Sphere destroys certain objects.  Returns
// true if the sphere would be cancelled; this isn't always used since
// this also gets called for exploding spheres.
static bool
object_destroyed(struct SphereState *sp) {
    struct Object obj = at(sp->x, sp->y)->obj;

    switch (obj.type) {
    case OHOME:
        // Destroying your home kills your family and ends the
        // game. So don't do that.
        say("You watch in horror as the Sphere of Annihilation reaches "
            "your home.\n");
        headsup();
        nap(1000);
        if (has_a(OSPHTALISMAN)) {
            say("You feel the Talisman vibrate.  The Sphere dissipates.\n");
            return true;
        } else {
            say("Your house is absorbed along with its occupants.\n");
            say("You can't bear to keep living.\n");
            game_over_probably(DDHOMEANNIH);
            return false;
        }// if .. else
        break;   // Not reached

    case OLRS:
        // Destroying the LRS office effectively bars you from
        // commerce because you won't be able to pay your bill.
        say("The Sphere destroys the LRS office.\n");
        say("The LRS adds the costs to your bill and freezes your assets.\n");
        UU.outstanding_taxes += 10000;
        break;

    case OSCHOOL: {
        if (graduated()) {
            // Did you know that universities can revoke your degree?
            // Well now you do.  The hard way.
            say("Just before its destruction, the University of Larn revokes "
                "your degree.\n");
            UU.courses[0] = false;

            // And delete the diploma if carried.
            int diplndx = index_of_first(ODIPLOMA);
            if (diplndx >= 0) {
                inventremove(diplndx);
            }// if
        }// if
        break;
    }

    }// switch

    return false;
}// object_destroyed


// Detonate sphere `sp`
static void
sphboom (struct SphereState *sp) {
    uint8_t x = sp->x, y = sp->y;

    if (UU.holdmonst) { UU.holdmonst = 1; }
    if (UU.cancellation) { UU.cancellation = 1; }

    const int left      = max(1, x - 2);
    const int right     = min(x + 3, MAXX - 1);
    const int top       = max(1, y - 2);
    const int bottom    = min(y + 3, MAXY - 1);

    for (int ix = left; ix < right; ix++) {
        for (int iy = top; iy < bottom; iy++) {

            struct SphereState boom = {ix, iy, sp->lev, sp->dir, 1};
            object_destroyed(&boom);

            if (UU.x == ix && UU.y == iy) {
                // If the player has the Talisman, they're immune...
                if (has_a(OSPHTALISMAN)) { return; }

                headsup();
                say("You were too close to the sphere!\n");
                nap(1000);
                game_over_probably(DDSPHERE); /* player killed in explosion */
            }/* if */

            at(ix, iy)->obj = obj(OANNIHILATION, 0);
            at(ix, iy)->mon = NULL_MON;
            see_and_update_at(ix, iy);
        }/* for */
    }// for

    nap(1000);

    for (int ix = left; ix < right; ix++) {
        for (int iy = top; iy < bottom; iy++) {
            at(ix, iy)->obj = NULL_OBJ;
            see_and_update_at(ix, iy);
        }// for
    }// for
}/* sphboom */


// Sphere interactions that only happen if the player isn't carrying
// the Talisman.  Returns true if the sphere has been/needs to be
// cancelled.
static bool
sphere_failed_no_talisman(struct SphereState *sp) {
    // Convenience shorthand
    int x = sp->x, y = sp->y;

    /* collision of spheres detonates spheres */
    if (at(x, y)->obj.type == OANNIHILATION) {
        say("Two spheres of annihilation collide! You hear a great "
            "earth-shaking blast!\n");
        headsup();
        sphboom(sp);   /* blow up stuff around sphere */
        return true;
    }// if

    struct Monster mon = at(x, y)->mon;

    /* demons dispel spheres */
    if (isdemon(mon)) {

        // Briefly show the demon.  (This bypasses some of the vision
        // checks, which is sort of wrong but I'm going to argue that
        // the Sphere is psionic in nature and so detectable even to
        // blind players.)
        say("The %s dispels the sphere!\n", monname_mon(mon));
        headsup();
        flash_at(x, y, monchar_mon(mon), 2000);

        return true;
    }// if

    /* disenchantress cancels spheres */
    if (mon.id == DISENCHANTRESS) {
        say("The %s cancels the sphere. It explodes!\n", monname_mon(mon));
        headsup();
        sphboom(sp);   /* blow up stuff around sphere */

        return true;
    }

    /* cancellation cancels spheres */
    if (UU.cancellation) {
        say("As the cancellation takes effect, you hear a great earth-shaking "
            "blast!\n");
        headsup();
        sphboom(sp);   /* blow up stuff around sphere */

        return true;
    }

    /* collision of sphere and player! */
    if (UU.x == x && UU.y == y) {
        say("You have been enveloped by the zone of nothingness!\n\n");
        headsup();
        nap(4000);
        game_over_probably(DDSELFANNIH);

        return true;
    }

    return false;
}// sphere_failed_no_talisman



// Test if sphere movement (or creation) fails for some reason and
// perform the related actions.
static bool
sphere_failed(struct SphereState *sp) {

    // Handle object destruction special cases, quitting if the sphere
    // was cancelled.
    bool cancel = object_destroyed(sp);
    if (cancel) { return true; }

    // Stuff that having the Talisman will bypass:
    if (!has_a(OSPHTALISMAN)) {
        return sphere_failed_no_talisman(sp);
    }

    // Otherwise, the Sphere Must Go On!
    return false;
}// sphere_failed


// Adjust the coordinates of sph so that they stay within the bounds
// of the current level.
static void
clip_pos(struct SphereState *sph) {
    int mx = MAXX - 2, my = MAXY - 2, mindim = 1;

    // On all but the first floor, the Sphere must be restricted to
    // inside the outer walls.  The upper floor has none, so we expand
    // the range by 1 on all directions.
    if (sph->lev == 0) {
        ++mx;
        ++my;
        --mindim;
    }// if

    sph->x = max(mindim, min(sph->x, mx));
    sph->y = max(mindim, min(sph->y, my));
}// clip_pos



// Move the sphere to destX,destY, destroying stuff and updating the
// display.  Also used to place the initial sphere.  Return true on
// success, false if the sphere needs to be deleted.
static bool
move_sphere(struct SphereState *sph, int destX, int destY) {

    // Remove the sphere object from the current location
    at(sph->x, sph->y)->obj     = NULL_OBJ;
    see_and_update_at(sph->x, sph->y);

    // Set the new location
    sph->x = destX;
    sph->y = destY;
    clip_pos(sph);

    // Handle the case where there's something, uh, interesting there.
    if (sphere_failed(sph)) {
        sph->lifetime = 0;
        return false;
    }// if

    // Place the sphere object and destroy whatever creature may be
    // there.
    at(sph->x, sph->y)->obj       = obj(OANNIHILATION, 0);
    at(sph->x, sph->y)->mon       = NULL_MON;
    see_and_update_at(sph->x, sph->y);

    return true;
}// move_sphere



// Find the first unused sphere slot and return a pointer to it (or to
// null, if they're all active.)
static struct SphereState *
find_slot(struct SphereList *list) {
    for (unsigned n = 0; n < MAX_SPHERES; n++) {
        if (list->spheres[n].lifetime == 0) {
            return &list->spheres[n];
        }
    }// for

    return NULL;
}// find_slot



// Create a new Sphere of Annihilation and add it to `list`.
void
newsphere(struct SphereList *list, int x, int y, DIRECTION dir) {

    // Add the new sphere to the linked list
    struct SphereState *sp = find_slot(list);
    if (!sp) {
        say("Your sphere of annihilation begin to form but then dissipates.\n");
        return;
    }// if

    *sp = (struct SphereState){x, y, getlevel(), dir, rnd(20) + 11};
    clip_pos(sp);

    // Place the sphere
    move_sphere(sp, sp->x, sp->y);

    say("A sphere of annihilation appears and begins to move.\n");
    return;
}/* newsphere */



// Deletes the Sphere(s) at x, y in the current level if present.  This
// is used for creatures (and presumably other in-game entities) that
// can dispel a Sphere
void
rmsphere(struct SphereList *list, int x, int y) {
    int currlevel = getlevel();

    // First, delete the Sphere(s) from the list.
    for (unsigned n = 0; n < MAX_SPHERES; n++) {
        struct SphereState *sp = &list->spheres[n];
        if (sp->lifetime && sp->x == x && sp->y == y && sp->lev == currlevel) {
            sp->lifetime = 0;
            break;
        }// if
    }// for

    // Now, delete it from the map.  (Note that this will work even if
    // the sphere isn't in UU.spheres.)
    if (at(x, y)->obj.type == OANNIHILATION) {
        at(x, y)->obj = NULL_OBJ;
        see_and_update_at(x,y);
    }// if
}// rmsphere



void
updatespheres(struct SphereList *spheres) {
    int lvl = getlevel();
    if (lvl < 0) { return; }    // Probably can't happen

    for (unsigned n = 0; n < MAX_SPHERES; n++) {
        struct SphereState *sp = &spheres->spheres[n];

        // If not active, or not on the current level, move on to the
        // next one.
        if (!sp->lifetime || sp->lev != lvl) {
            continue;
        }// if 

        // Decrement the lifetime and delete the sphere if it reaches
        // zero (i.e. the sphere has dissipated).
        --sp->lifetime;
        if (sp->lifetime == 0) {
            at(sp->x, sp->y)->obj = NULL_OBJ;
            see_and_update_at(sp->x, sp->y);
            continue;
        }// if

        // Change direction, if the random number generator decrees it.
        if (rnd( max(7, intelligence()/2) ) <= 2) {
            sp->dir = randdir();
            continue;
        }

        // Otherwise, move the sphere one spot
        int8_t nx, ny;
        adjpoint(sp->x, sp->y, sp->dir, &nx, &ny);
        bool success = move_sphere(sp, nx, ny);

        // If failed, delete all spheres at this location.  (This is
        // because when spheres collide, they should both be deleted.)
        if (!success) {
            rmsphere(spheres, sp->x, sp->y);
        }// if
    }// for

}// updatespheres
