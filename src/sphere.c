// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "display.h"
#include "ui.h"
#include "game.h"

#include "sphere.h"

/*
 *
 *  rmsphere(x,y)       delete a sphere of annihilation from list
 *      int x,y;
 *
 *  sphboom(x,y)        perform the effects of a sphere detonation
 *      int x,y;
 */


static bool
sphere_failed(int x, int y) {

    // The Talisman of the Sphere bypasses all of these problems
    if (has_a(OSPHTALISMAN)) { return false; }

    /* collision of spheres detonates spheres */
    if (Map[x][y].obj.type == OANNIHILATION) {
        say("Two spheres of annihilation collide! You hear a great "
            "earth-shaking blast!\n");
        headsup();
        rmsphere(x,y);
        sphboom(x,y);   /* blow up stuff around sphere */

        return true;
    }// if

    int mon = Map[x][y].mon.id;

    /* demons dispel spheres */
    if (mon >= DEMONLORD1) {

        /* show the demon (ha ha) */
        int i = Map[x][y].mon.hitp;
        Map[x][y].know = 1;
        show1cell(x,y);

        say("The %s dispels the sphere!\n", MonType[mon].name);

        headsup();
        rmsphere(x,y);  /* remove any spheres that are here */

        // Restore the demon
        Map[x][y].mon.id = mon;
        Map[x][y].mon.hitp = i;
        Map[x][y].know = 0;

        return true;
    }// if

    /* disenchantress cancels spheres */
    if (mon == DISENCHANTRESS) {
        say("The %s cancels the sphere!\n", MonType[mon].name);
        headsup();
        sphboom(x,y);   /* blow up stuff around sphere */
        rmsphere(x,y);  /* remove any spheres that are here */

        return true;
    }

    /* cancellation cancels spheres */
    if (UU.cancellation) {
        say("As the cancellation takes effect, you hear a great earth-shaking "
            "blast!\n");
        headsup();
        sphboom(x,y);   /* blow up stuff around sphere */
        rmsphere(x,y);  /* remove any spheres that are here */

        return true;
    }

    /* collision of sphere and player! */
    if (UU.x == x && UU.y == y) {
        say("You have been enveloped by the zone of nothingness!\n\n");
        headsup();
        rmsphere(x,y);  /* remove any spheres that are here */
        nap(4000);
        game_over_probably(DDSELFANNIH);

        return true;
    }

    return false;
}// sphere_failed



/*
 *  Function to create a new sphere of annihilation.
 *
 *  Enter with the coordinates of the sphere in (x,y), the direction
 *  (0-8 diroffx format) in dir, and the lifespan of the sphere (in
 *  turn) in life.
 *  
 *  Returns the number of spheres currently in existence.
 */
void
newsphere (int x, int y, int dir, int life) {
    if (dir>=9) { dir=0; }  /* no movement if direction not found */

    /* don't go out of bounds */
    if (getlevel() == 0) {
        VXY(x,y);       // Only level 0 doesn't have outer walls
    } else {
        x = max(1, min(x, MAXX-2));
        y = max(1, min(y, MAXY-2));
    }

    if (sphere_failed(x, y)) { return; }

    // Annihilate the location
    Map[x][y].obj       = obj(OANNIHILATION, 0);
    Map[x][y].mon.id    = 0;
    Map[x][y].know      = 1;

    // show the new sphere
    show1cell(x,y);

    // Add the new sphere to the linked list
    struct sphere *sp = xmalloc(sizeof(struct sphere));
    *sp = (struct sphere){spheres, x, y, getlevel(), dir, life};
    spheres = sp;

    // And up the number of spheres in play.
    UU.sphcast++;
}/* newsphere */


/*
 *  Function to delete a sphere of annihilation from list.
 *
 *  Enter with the coordinates of the sphere (on current level).
 */
void
rmsphere (int x, int y) {
    struct sphere *sp,*sp2=(struct sphere *)NULL;

    for (sp=spheres; sp; sp2=sp, sp=sp->p) {

        /* is sphere on this level? */
        if (getlevel() == sp->lev) {
            
            /* locate sphere at this location */
            if ((x==sp->x) && (y==sp->y)) {
                Map[x][y].obj.type= Map[x][y].mon.id= 0;
                Map[x][y].know=1;
                show1cell(x,y); /* show the now missing sphere */
                --UU.sphcast;
                if (sp==spheres) {
                    sp2=sp;
                    spheres=sp->p;
                    if (sp2) {
                        free(sp2); }
                }
                else {
                    sp2->p = sp->p;
                    if (sp)
                        free(sp);
                }
                break;
            }// if
        }// if
    }// for 
}/* rmsphere */


/*
 *  Function to perform the effects of a sphere detonation
 *
 *  Enter with the coordinates of the blast, Returns no value
 */
void
sphboom (int x, int y) {
    int i,j,k;

    if (UU.holdmonst) UU.holdmonst=1;
    if (UU.cancellation) UU.cancellation=1;
    for (j=max(1,x-2); j<min(x+3,MAXX-1); j++)
        for (i=max(1,y-2); i<min(y+3,MAXY-1); i++) {
            Map[j][i].obj = NullObj;
            Map[j][i].mon = NullMon;
            show1cell(j,i);
            for (k=0;k<IVENSIZE;k++)
                if (Invent[k].type==OSPHTALISMAN)
                    return;
            if (UU.x==j && UU.y==i) {
                headsup();
                say("You were too close to the sphere!\n");
                nap(3000);
                game_over_probably(DDSPHERE); /* player killed in explosion */
            }/* if */
        }/* for */
}/* sphboom */


/*
 *  movsphere()     Function to look for and move spheres of annihilation
 *
 * This function works on the sphere linked list, first duplicating the list
 * (the act of moving changes the list), then processing each sphere in order
 * to move it.  They eat anything in their way, including stairs, volcanic
 * shafts, potions, etc, except for upper level demons, who can dispel
 * spheres.
 */
#define SPHMAX 20   /* maximum number of spheres movsphere can handle */
void
movsphere(void) {
    int x,y,dir,len;
    struct sphere *sp,*sp2;
    struct sphere sph[SPHMAX];

    /* first duplicate sphere list */
    /* look through sphere list */
    for (x=0,sp2=spheres; sp2; sp2=sp2->p)
        /* only if this level */
        if (sp2->lev == getlevel()) {
            sph[x] = *sp2;
            sph[x++].p = (struct sphere *)NULL;  /* copy the struct */
            if (x>1)
                sph[x-2].p = &sph[x-1]; /* link pointers */
        }

    if (x) sp= sph; /* if any spheres, point to them */
    else return;    /* no spheres */

    /* look through sphere list */
    for (sp=sph; sp; sp=sp->p) {
        x = sp->x;
        y = sp->y;
        if (Map[x][y].obj.type!=OANNIHILATION)
            continue;/* not really there */

        /* has sphere run out of gas? */
        if (--(sp->lifetime) < 0) {
            rmsphere(x,y); /* delete sphere */
            continue;
        }

        /* time to move the sphere */
        int action = rnd( max(7, UU.intelligence/2) );
        if (action <= 2) {
            /* change direction to a random one */
            sp->dir = rnd(8);
        } else {
            /* move in normal direction */
            dir = sp->dir;
            len = sp->lifetime;

            int nx, ny;
            adjpoint(x, y, dir, &nx, &ny);

            rmsphere(x, y);
            newsphere(nx, ny, dir, len);
        }// if .. else
    }// for

}/* movsphere*/
