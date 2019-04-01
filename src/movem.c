// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "internal_assert.h"
#include "sphere.h"

#include "display.h"

#include "movem.h"

static void mmove(int xsrc, int ysrc, int xdest, int ydest);
static void movemt(int x, int y, int winleft, int winright, int wintop,
                   int winbot, int distance);
static void setmovewin(int *top, int *bot, int *left, int *right,
                       int *distance);
static void clearmoved(int mvtop, int mvbot, int mvleft, int mvright);



/* Code to set the coordinates of the last monster hit.

   This monster moves even if it's outside the usual monster movement
   window.  In addition, mmove() calls lasthit() on the new location
   so that it will (probably) move on next turn as well.

   TODO: the downside of this is that there can only be one angry
   monster per level.  Ideally, we should keep a list and add each hit
   monster to it.

 */
static uint8_t lasthx = 0, lasthy = 0;
static uint8_t lastlevel = -1; /* coords invalid unless lastlevel==curr lvl */

void
lasthit(uint8_t x, uint8_t y) {
    lasthx = x;
    lasthy = y;
    lastlevel = getlevel();
}/* lasthit*/

static bool
lasthit_valid() {
    return lastlevel == getlevel() && Map[lasthx][lasthy].mon.id != NOMONST;
}/* lasthit_valid*/

static bool
is_lasthit(int x, int y) {
    return lastlevel == getlevel() && x == lasthx && y == lasthy;
}/* is_lasthit*/


/*
 *  movemonst()     Routine to move the monsters toward the player
 *
 *  This routine has the responsibility to determine which monsters are to
 *  move, and call movemt() to do the move.
 *  Returns no value.
 */
void
movemonst() {
    int x, y;
    int mvtop, mvbot, mvleft, mvright, distance;

    /* no action if time is stopped */
    if (UU.timestop) return;

    /* Skip alterate turns if the user is fast. */
    if (UU.hasteSelf && (UU.hasteSelf & 1) == 0)  return;

    /* move the spheres of annihilation if any */
    if (spheres) {
        movsphere();
    }/* if */

    /* no action if monsters are held */
    if (UU.holdmonst) return;

    /* determine window of monsters to move */
    setmovewin(&mvtop, &mvbot, &mvleft, &mvright, &distance);

    /* Clear the moved flags */
    clearmoved(mvtop, mvbot, mvleft, mvright);

    /* Move the monsters in the window. */
    for (y = mvtop; y < mvbot; y++) {
        for (x = mvleft; x < mvright; x++) {

            /* We only care about squares with a monster that hasn't
             * moved yet. */
            if (!Map[x][y].mon.id || Map[x][y].mon.moved) continue;

            /* Move the monster unless it's sleeping, player is
             * stealthed and non-aggraviting. */
            if (Map[x][y].mon.awake || UU.aggravate || !UU.stealth) {
                movemt(x, y, mvleft, mvright, mvtop, mvbot, distance);
            }
        }/* for */
    }/* for */

    /* Move the last hit (i.e. angry) monster if present and currently
     * unmoved. */
    if (lasthit_valid() && !Map[lasthx][lasthy].mon.moved) {
        movemt(lasthx, lasthy, mvleft, mvright, mvtop, mvbot, distance);
    }/* if */
}/* movemonst*/


static void
setmovewin(int *top, int *bot, int *left, int *right, int *distance) {

    /* Set the initial window. */
    *top       = UU.y-3;
    *bot       = UU.y+4;
    *left      = UU.x-5;
    *right     = UU.x+6;
    *distance  = 17;

    /* The window is bigger if monsters are aggravated. */
    if (UU.aggravate) {
        *top        = UU.y-5;
        *bot        = UU.y+6;
        *left       = UU.x-10;
        *right      = UU.x+11;
        *distance   = 40;
    }/* if */

    /* Constrain the window to the map. */
    VXY(*left, *top);
    VXY(*right, *bot);

    /* And skip the outer wall on levels that have it (i.e. everything
     * except the town. */
    if (getlevel() != 0) {
        *top   = max(*top, 1);
        *bot   = min(*bot, MAXY - 1);
        *left  = max(*left, 1);
        *right = min(*right, MAXX - 1);
    }/* if */

}/* setmovewin*/


/* Clear the moved flags. */
static void
clearmoved(int mvtop, int mvbot, int mvleft, int mvright) {
    int x, y;
    for (y=mvtop; y<mvbot; y++) {
        for (x=mvleft; x<mvright; x++) {
            Map[x][y].mon.moved = 0;
        }
    }/* for */

    Map[lasthx][lasthy].mon.moved=0;
}/* clearmoved*/


/*  If the monster at (x,y) is scared, move randomly.
 *
 *  Return true if the monster moved (i.e. it was scared), false if
 *  not. */
static bool
movescared(int x, int y) {
    int fear, xdest, ydest;
    uint8_t dstobjtp;
    int mon_id = Map[x][y].mon.id;

    /* Determine if the monster is scared. */
    fear = has_a(OHANDofFEAR) ? 1 : 0;
    if (fear && rnd(10) > 4) fear=0;
    if (fear && UU.scaremonst) fear += 1;
    if (mon_id > DEMONLORD1 || mon_id == PLATINUMDRAGON) {
        fear = (fear==1) ? 0 : (rnd(10) > 5);
    }/* if */

    if (!fear) return false;

    /* Pick a random direction to move. */
    xdest = x + rnd(3) - 2;
    ydest = y + rnd(3) - 2;
    VXY(xdest, ydest);

    /* Bail if the destination is unsuitable for some reason. */
    dstobjtp = Map[xdest][ydest].obj.type;
    if (dstobjtp == OWALL)                          return true;
    if (Map[xdest][ydest].mon.id != 0)              return true;
    if (mon_id == VAMPIRE && dstobjtp != OMIRROR)   return true;
    if (dstobjtp == OCLOSEDDOOR)                    return true;

    /* Make the move. */
    mmove(x, y, xdest, ydest);

    return true;
}/* movescared*/


// If the monster at (x,y) is smart, move it intelligently.
//
// Return true if the monster is sufficiently smart and thus moved,
// false if not.
static bool
movesmart(const int x, const int y, const int winleft, const int winright,
          const int wintop, const int winbot, const int distance) {
    static short screenbuf[MAXX][MAXY];
    int xl, yl, xh, yh, ypt, xpt, tier;
    const uint8_t mon_id = Map[x][y].mon.id;

    /* Bail if this monster isn't smart enough. */
    if (MonType[mon_id].intelligence <= 10-UU.challenge) return false;

    /* Compute the range of mapsquares to examine. */
    xl=winleft  - 2;
    yl=wintop   - 2;
    xh=winright + 2;
    yh=winbot   + 2;
    VXY(xl,yl);
    VXY(xh,yh);

    /* Initialize screenbuf, marking all inaccessible places with 127
     * and the rest with 0 (meaning unset). */
    for (ypt=yl; ypt <= yh; ypt++) {
        for (xpt=xl; xpt <= xh; xpt++) {

            /* Demon princes aren't afraid of anything. */
            if (mon_id >= DEMONPRINCE) {
                screenbuf[xpt][ypt]=0;
                continue;
            }/* if */

            /* Vampires don't like mirrors.  (The original code only
             * avoided the mirror if there was a vampire standing on
             * it.  I'm assuming that's a bug and fixing it.) */
            if (mon_id == VAMPIRE && Map[xpt][ypt].obj.type == OMIRROR) {
                screenbuf[xpt][ypt]=127;
                continue;
            }/* if */

            switch(Map[xpt][ypt].obj.type) {
            case OWALL:
            case OELEVATORUP:
            case OELEVATORDOWN:
            case OPIT:
            case OTRAPARROW:
            case ODARTRAP:
            case OCLOSEDDOOR:
            case OTRAPDOOR:
            case OTELEPORTER:
            case OEXIT:
                screenbuf[xpt][ypt]=127;
                break;

            default:
                screenbuf[xpt][ypt]=0;
                break;
            }/* switch*/

        }/* for */
    }/* for */

    /* The player's location is tier 1. */
    screenbuf[UU.x][UU.y] = 1;

    /* Now, find a path from the monster to the player. Starting at
     * the player's tier (1), we search for squares with the given
     * tier and mark all adjacent squares with the next tier's value.
     * We repeat this for increasing tier values until either we reach
     * the monster or we exceed 'distance'. */
    xl = winleft  - 1;
    yl = wintop   - 1;
    xh = winright + 1;
    yh = winbot   + 1;
    VXY(xl,yl);
    VXY(xh,yh);

    for (tier = 1; tier < distance; tier++) {    /* only up to 20 squares away */
        for (ypt = yl; ypt < yh; ypt++) {
            for (xpt = xl; xpt < xh; xpt++) {
                if (screenbuf[xpt][ypt] == tier) {
                    int z;

                    /* We've found a spot at this tier.  Mark adjacent
                     * squares with the next tier. */
                    for (z = 1; z < 9; z++) {
                        int xtmp, ytmp;
                        adjpoint(xpt, ypt, z, &xtmp, &ytmp);

                        if (screenbuf[xtmp][ytmp] == 0) {
                            screenbuf[xtmp][ytmp] = tier + 1;
                        }/* if */

                        /* Quit if we've reached the monster. */
                        if (xtmp==x && ytmp==y) goto exit_tier_loops;
                    }/* for */
                }/* if */
            }/* for */
        }/* for */
    }/* for */
exit_tier_loops:

    /* If we found a path, take it. */
    if (tier < distance) {
        int z, bestx, besty;
        int best = distance + 1;

        /* Search adjacent squares for the lowest tier value since
         * that will presumably be the shortest path to the player. */
        for (z = 1; z < 9; z++) {
            int xx, yy;
            adjpoint(x, y, z, &xx, &yy);

            if (screenbuf[xx][yy] > 0 && screenbuf[xx][yy] < best &&
                (xx != x || yy != y)) {
                best = screenbuf[xx][yy];
                bestx = xx;
                besty = yy;
            }/* if */
        }/* for */

        /* And if we found a best path, take it. */
        if (best < distance + 1) {
            mmove(x,y, bestx, besty);

            return true;
        }/* if */
    }/* if */

    return false;
}/* movesmart*/


// Move the monster at (x,y) as an unintelligent creature.
static void
movedumb(int x, int y) {
    int xl, xh, yl, yh, xpt, ypt;
    int bestd = MAXX*MAXY*3, bestx = -1, besty = -1;

    xl = x-1;
    yl = y-1;
    xh = x+2;
    yh = y+2;

    if (x < UU.x)
        xl++;
    else if (x > UU.x)
        --xh;

    if (y < UU.y)
        yl++;
    else if (y > UU.y)
        --yh;

    // This isn't quite right because the loop below excludes xh/yh so
    // the leftmost and bottommost rows are skipped.  Probably not a
    // big deal, but fix?
    VXY(xl, yl);
    VXY(xh, yh);

    /* for each square compute distance to player */
    for (xpt = xl; xpt < xh; xpt++) {
        for (ypt = yl; ypt < yh; ypt++) {
            int otype;

            otype = Map[xpt][ypt].obj.type;

            if (xpt == 33 && ypt == MAXY-1 && getlevel() == 1)
                continue;

            if (/* Not a wall (or the player in the wall). */
                (otype != OWALL || (xpt == UU.x && ypt == UU.y))     &&
                /* no monster already there */
                (Map[xpt][ypt].mon.id == 0)                         &&
                /* vampires not move towards mirrors */
                !(Map[x][y].mon.id == VAMPIRE && otype != OMIRROR)   &&
                /* not move towards closed door */
                (otype != OCLOSEDDOOR)
                ) {
                int dist = (UU.x-xpt)*(UU.x-xpt)+(UU.y-ypt)*(UU.y-ypt);

                if (dist < bestd) {
                    bestd = dist;
                    bestx = xpt;
                    besty = ypt;
                }/* if */
            }/* if */
        }/* for */
    }/* for */

    /* Move if we found the best direction. */
    if (bestx >= 0 && besty > 0) {
        mmove(x, y, bestx, besty);
    }/* if */
}/* movedumb*/


/*
 *  Function to move a monster at (x,y) -- must determine where.
 *
 *  This routine is responsible for determining where one monster at (x,y)
 *  will move to.  Enter with the monsters coordinates in (x,y).
 *  Returns no value.
 */
static void
movemt(int x, int y, int winleft, int winright, int wintop, int winbot,
       int distance) {
    struct Monster mon;

    mon = Map[x][y].mon;

    if (isslow(mon) && (UU.gtime & 1)) {
        return;
    }/* if */

    /* choose destination randomly if scared */
    if (movescared(x, y)) return;

    /* Move if monster is intelligent. */
    if (movesmart(x, y, winleft, winright, wintop, winbot, distance)) return;

    /* Otherwise, move it as dumb. */
    movedumb(x, y);
}/* movemt*/



/* Handle the monster at xdest, ydest if there's a pit there. */
static void
checkpit(int xdest, int ydest) {
    struct Object dob = Map[xdest][ydest].obj;
    struct Monster mon = Map[xdest][ydest].mon;

    if (!ismon(mon)) return;
    if (dob.type != OPIT && dob.type != OTRAPDOOR) return;
    if (avoidspits(mon)) return;

    Map[xdest][ydest].mon = NULL_MON;  /* fell in a pit or trapdoor */
}/* checkpit*/


/* Handle the monster at xdest, ydest if there's a Sphere of
 * Annilihation there. */
static void
checksphere(int xdest, int ydest) {
    struct Object dob = Map[xdest][ydest].obj;
    struct Monster mon = Map[xdest][ydest].mon;
    bool tos;

    if (!ismon(mon)) return;
    if (dob.type != OANNIHILATION) return;

    tos = has_a(OSPHTALISMAN);

    /* demons dispel spheres */
    if ((mon.id >= DEMONLORD1 && !tos) ||
        (tos && mon.id == DEMONKING && (rnd(10) > 7))) {
        say("The %s dispels the sphere!\n", MonType[mon.id].name);
        rmsphere(xdest,ydest);    /* delete the sphere */
        return;
    }/* if */

    /* Otherise, poof! */
    Map[xdest][ydest].mon = NULL_MON;
}/* checksphere*/


/* If the monster is a lemming, there's a chance that it reproduced on
 * move. */
static void
checklemming(int xdest, int ydest, int xsrc, int ysrc) {
    struct Monster mon = Map[xdest][ydest].mon;

    if (mon.id != LEMMING) return;

    /* 2% chance moving a lemming creates a new lemming in the old
     * spot.  (Was 10%, which is.... annoying.) */
    if (rnd(100) <= 2) {
        Map[xsrc][ysrc].mon = mon;
    }/* if */
}/* checklemming*/


/* If the monster is a leprechaun, pocket whatever valuables are here. */
static void
checkleprechaun(int xdest, int ydest) {
    struct Object dob = Map[xdest][ydest].obj;
    struct Monster mon = Map[xdest][ydest].mon;

    if (mon.id != LEPRECHAUN) return;
    if (isshiny(dob) && Lev->numStolen < (2*MAX_STOLEN)/3) {
        add_to_stolen (dob, Lev);
        Map[xdest][ydest].obj = NULL_OBJ;
    }/* if */
}/* checkleprechaun*/


 /* if a troll regenerate him */
static void
checktroll(int xdest, int ydest) {
    struct Monster mon = Map[xdest][ydest].mon;

    if (mon.id != TROLL) return;
    if (UU.gtime & 1) return;   /* Only every second turn. */

    if (maxhp(mon) > mon.hitp) {
        mon.hitp++;
        Map[xdest][ydest].mon = mon;
    }/* if */
}/* checktroll*/


/* Check for dart- or arrow-trap. */
static void
checkpointy(int xdest, int ydest) {
    struct Object dob = Map[xdest][ydest].obj;
    struct Monster mon = Map[xdest][ydest].mon;
    const char *who, *what;

    if (!ismon(mon)) return;
    if (dob.type != OTRAPARROW || dob.type != ODARTRAP) return;

    if (dob.type == OTRAPARROW) {
        who = "An arrow";
        mon.hitp -= rnd(10)+getlevel();
    } else {
        who = "A dart";
        mon.hitp -= rnd(6);
    }/* if .. else*/

    if (mon.hitp <= 0) {
        mon = NULL_MON;
        what = "%s hits the %s.\n";
    } else {
        what = "%s hits and kills the %s.\n";
    }/* if .. else*/

    Map[xdest][ydest].mon = mon;

    /* Print the message. */
    if (!UU.blindCount) {
        say(what, who, monname_mon(mon));
    }/* if */
}/* checkpointy*/


/* Check for elevator or teleport. */
static void
checkzapper(int xdest, int ydest) {
    struct Object dob = Map[xdest][ydest].obj;
    struct Monster mon = Map[xdest][ydest].mon;
    const char *what;

    if (!ismon(mon) || isdemon(mon)) return;

    if (dob.type == OTELEPORTER) {
        what = "The %s gets teleported.\n";
        teleportmonst(xdest, ydest);
    } else if (dob.type == OELEVATORDOWN || dob.type == OELEVATORUP) {
        what = "The %s is carried away by an elevator!\n";
        Map[xdest][ydest].mon = NULL_MON;
    } else {
        return;
    }/* if .. else*/

    if (!UU.blindCount) {
        say(what, monname_mon(mon));
    }/* if */
}/* checkzapper*/



/* Actually move the monster at (xsrc, ysrc) to (xdest, ydest),
 * triggering any related action.  If the player is at (xdest, ydest),
 * this becomes an attack instead. */
static void
mmove(int xsrc, int ysrc, int xdest, int ydest) {

    /* Case 1: moving to player's square attacks him/her. */
    if ((xdest==UU.x) && (ydest==UU.y)) {
        hitplayer(xsrc,ysrc);
        Map[xsrc][ysrc].mon.moved = 1;
        return;
    }

    /* Actually move the monster. */
    Map[xdest][ydest].mon = Map[xsrc][ysrc].mon;
    Map[xdest][ydest].mon.moved = 1;
    Map[xdest][ydest].mon.awake = 1;
    Map[xsrc][ysrc].mon = NULL_MON;

    /* If this is the monster that was last hit, keep it angry. */
    if (is_lasthit(xsrc, ysrc)) {
        lasthit(xdest, ydest);
    }/* if */

    /* Handle the various special side-effects of moving. */
    checkpit(xdest, ydest);
    checksphere(xdest, ydest);
    checklemming(xdest, ydest, xsrc, ysrc);
    checkleprechaun(xdest, ydest);
    checktroll(xdest, ydest);
    checkpointy(xdest, ydest);
    checkzapper(xdest, ydest);
}/* mmove*/


void
teleportmonst (int srcx, int srcy) {
    int destx, desty, trys;
    int monid;

    /* max # of creation attempts */
    for (trys = 10; trys > 0; --trys) {
        destx = rnd(MAXX-2);
        desty = rnd(MAXY-2);

        if ((Map[destx][desty].obj.type == 0)
            && (Map[destx][desty].mon.id == 0)
            && ((UU.x != destx) || (UU.y != desty))) {
            break;
        }/* if */
    }/* for */

    if (!trys) {
        return;
    }/* if */

    ASSERT (Map[destx][desty].obj.type == 0 &&
            Map[destx][desty].mon.id == 0);

    monid = Map[srcx][srcy].mon.id;

    Map[destx][desty].mon = Map[srcx][srcy].mon;
    Map[destx][desty].mon.hitp = mon_hp(monid);
    see_at(destx, desty);

    Map[srcx][srcy].mon = NULL_MON;

    return;
}/* teleportmonst */
