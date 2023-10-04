// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "internal_assert.h"
#include "sphere.h"
#include "player.h"
#include "ui.h"

#include "display.h"

#include "movem.h"

#include <limits.h>

static void mmove(int8_t xsrc, int8_t ysrc, int8_t xdest, int8_t ydest);
static void movemt(int8_t x, int8_t y, int8_t winleft, int8_t winright, int8_t wintop,
                   int8_t winbot, int8_t distance);
static void setmovewin(int8_t *top, int8_t *bot, int8_t *left, int8_t *right,
                       int8_t *distance);
static void clearmoved(int8_t mvtop, int8_t mvbot, int8_t mvleft, int8_t mvright);



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
    return lastlevel == getlevel() && at(lasthx, lasthy)->mon.id != NOMONST;
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
    int8_t mvtop, mvbot, mvleft, mvright, distance;

    /* no action if time is stopped */
    if (UU.timestop) return;

    /* Skip alterate turns if the user is fast. */
    if (UU.hasteSelf && (UU.hasteSelf & 1) == 0)  return;

    /* move the spheres of annihilation if any */
    updatespheres(&UU.spherelist);

    /* no action if monsters are held */
    if (UU.holdmonst) return;

    /* determine window of monsters to move */
    setmovewin(&mvtop, &mvbot, &mvleft, &mvright, &distance);

    /* Clear the moved flags */
    clearmoved(mvtop, mvbot, mvleft, mvright);

    /* Move the monsters in the window. */
    for (int y = mvtop; y <= mvbot; y++) {
        for (int x = mvleft; x <= mvright; x++) {

            /* We only care about squares with a monster that hasn't
             * moved yet. */
            if (!at(x, y)->mon.id || at(x, y)->mon.moved) continue;

            /* Move the monster unless it's sleeping, player is
             * stealthed and non-aggraviting. */
            if (at(x, y)->mon.awake || UU.aggravate || !UU.stealth) {
                movemt(x, y, mvleft, mvright, mvtop, mvbot, distance);
            }
        }/* for */
    }/* for */

    /* Move the last hit (i.e. angry) monster if present and currently
     * unmoved. */
    if (lasthit_valid() && !at(lasthx, lasthy)->mon.moved) {
        movemt(lasthx, lasthy, mvleft, mvright, mvtop, mvbot, distance);
    }/* if */
}/* movemonst*/


static void
setmovewin(int8_t *top, int8_t *bot, int8_t *left, int8_t *right, int8_t *distance) {

    // Aggravation increases the window size
    const uint8_t xdiff = UU.aggravate ? 10 : 5;
    const uint8_t ydiff = UU.aggravate ? 3  : 5;

    // Set the window
    *top       = UU.y - ydiff;
    *bot       = UU.y + ydiff + 1;
    *left      = UU.x - xdiff;
    *right     = UU.x + xdiff + 1;

    // Set the distance
    *distance = UU.aggravate ? 40 : 17;

    /* Constrain the window to the map. */
    clip(left, top);
    clip(right, bot);
}/* setmovewin*/


/* Clear the moved flags. */
static void
clearmoved(int8_t mvtop, int8_t mvbot, int8_t mvleft, int8_t mvright) {
    for (int8_t y = mvtop; y <= mvbot; y++) {
        for (int8_t x = mvleft; x <= mvright; x++) {
            at(x, y)->mon.moved = 0;
        }
    }/* for */

    at(lasthx, lasthy)->mon.moved=0;
}/* clearmoved*/


/*  If the monster at (x,y) is scared, move randomly.
 *
 *  Return true if the monster moved (i.e. it was scared), false if
 *  not. */
static bool
movescared(int x, int y) {
    uint8_t dstobjtp;
    int mon_id = at(x, y)->mon.id;

    /* Determine if the monster is scared. */
    int fearlvl = (int)has_a(OHANDofFEAR);
    if (fearlvl && rnd(10) > 4)     fearlvl = 0;
    if (fearlvl && UU.scaremonst)   fearlvl += 1;
    if (mon_id > DEMONLORD1 || mon_id == PLATINUMDRAGON) {
        fearlvl = (fearlvl == 1) ? 0 : (rnd(10) > 5);
    }/* if */

    if (fearlvl <= 0) return false;

    /* Pick a random direction to move. */
    int8_t xdest = x + rnd(3) - 2;
    int8_t ydest = y + rnd(3) - 2;
    clip(&xdest, &ydest);

    /* Bail if the destination is unsuitable for some reason. */
    dstobjtp = at(xdest, ydest)->obj.type;
    if (dstobjtp == OWALL)                          return true;
    if (at(xdest, ydest)->mon.id != 0)              return true;
    if (mon_id == VAMPIRE && dstobjtp != OMIRROR)   return true;
    if (dstobjtp == OCLOSEDDOOR)                    return true;

    /* Make the move. */
    mmove(x, y, xdest, ydest);

    return true;
}/* movescared*/


/* Initialize pathbuf (or at least, the monster's window of movement
 * inside pathbuf plus a bit more), marking all inaccessible places
 * with INT8_MAX and the rest with 0 (meaning unset). */
static void
movesmart_init_pathbuf(int8_t pathbuf[MAXX][MAXY], const uint8_t mon_id,
                       const int8_t winleft, const int8_t winright,
                       const int8_t wintop, const int8_t winbot) {

    /* Compute the range of mapsquares to examine. */
    int8_t xl = winleft  - 2;
    int8_t yl = wintop   - 2;
    int8_t xh = winright + 2;
    int8_t yh = winbot   + 2;
    clip(&xl, &yl);
    clip(&xh, &yh);

    for (int8_t ypt = yl; ypt <= yh; ypt++) {
        for (int8_t xpt = xl; xpt <= xh; xpt++) {

            /* Demon princes aren't afraid of anything. */
            if (mon_id >= DEMONPRINCE) {
                pathbuf[xpt][ypt] = 0;
                continue;
            }/* if */

            /* Vampires don't like mirrors.  (The original code only
             * avoided the mirror if there was a vampire standing on
             * it.  I'm assuming that's a bug and fixing it.) */
            if (mon_id == VAMPIRE && at(xpt, ypt)->obj.type == OMIRROR) {
                pathbuf[xpt][ypt] = INT8_MAX;
                continue;
            }/* if */

            switch(at(xpt, ypt)->obj.type) {
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
                pathbuf[xpt][ypt] = INT8_MAX;
                break;

            default:
                pathbuf[xpt][ypt] = 0;
                break;
            }/* switch*/

        }/* for */
    }/* for */
}// movesmart_init_pathbuf


// Find a path from the monster to the player.
//
// We do this by iteratively setting cells in pathbuf to values that
// correspond to the distance from the player.  Starting from 1 (which
// the player's cell is set to) we iteratively search for all cells of
// the given value and set all adjacent unset cell values to the next
// value before repeating the process for that value.  The process
// ends when the monster is reached or the count reaches 'distance'.
//
// Returns true if the monster was found, false otherwise.
static bool
movesmart_fill_distance(int8_t pathbuf[MAXX][MAXY],
                        const int8_t x, const int8_t y,
                        const int8_t winleft, const int8_t winright,
                        const int8_t wintop, const int8_t winbot,
                        const int8_t distance) {

    /* The player's location is tier 1. */
    pathbuf[UU.x][UU.y] = 1;

    // Compute the search window
    int8_t xl = winleft  - 1;
    int8_t yl = wintop   - 1;
    int8_t xh = winright + 1;
    int8_t yh = winbot   + 1;
    clip(&xl, &yl);
    clip(&xh, &yh);

    // Now, iteratively find all cells with the current tier value and
    // set all adjacent cells that are currently zero to the next tier
    // value.  This floods an ever-increasing series of numbers toward
    // the monster.
    for (int8_t tier = 1; tier < distance; tier++) {
        for (int8_t ypt = yl; ypt < yh; ypt++) {
            for (int8_t xpt = xl; xpt < xh; xpt++) {
                if (pathbuf[xpt][ypt] == tier) {
                    int z;

                    /* We've found a spot at this tier.  Mark adjacent
                     * squares with the next tier. */
                    for (z = DIR_MIN_DIR; z <= DIR_MAX ; z++) {
                        int8_t xtmp, ytmp;
                        adjpoint(xpt, ypt, z, &xtmp, &ytmp);

                        if (pathbuf[xtmp][ytmp] == 0) {
                            pathbuf[xtmp][ytmp] = tier + 1;
                        }/* if */

                        /* Quit if we've reached the monster. */
                        if (xtmp==x && ytmp==y) { return true; }
                    }/* for */
                }/* if */
            }/* for */
        }/* for */
    }/* for */

    // If we get here, we failed
    return false;
}// movesmart_fill_distance

// Move the monster using the weight values in pathbuf.
static bool
movesmart_do_move(int8_t pathbuf[MAXX][MAXY], int8_t x, int8_t y,
                  int8_t distance) {
    int best = distance + 1;

    /* Search adjacent squares for the lowest tier value since
     * that will presumably be the shortest path to the player. */
    int8_t bestx = 0, besty = 0;
    for (int z = DIR_MIN_DIR; z <= DIR_MAX; z++) {
        int8_t xx, yy;
        adjpoint(x, y, z, &xx, &yy);

        if (pathbuf[xx][yy] > 0 && pathbuf[xx][yy] < best &&
            (xx != x || yy != y)) {
            best = pathbuf[xx][yy];
            bestx = xx;
            besty = yy;
        }/* if */
    }/* for */

    /* And if we found a best path, take it. */
    if (best < distance + 1) {
        mmove(x,y, bestx, besty);
        return true;
    }/* if */

    return false;
}// movesmart_do_move


// If the monster at (x,y) is smart, move it intelligently.
//
// Return true if the monster is sufficiently smart and thus moved,
// false if not.
static bool
movesmart(const int8_t x, const int8_t y,
          const int8_t winleft, const int8_t winright,
          const int8_t wintop, const int8_t winbot,
          const int8_t distance) {
    const uint8_t mon_id = at(x, y)->mon.id;

    /* Bail if this monster isn't smart enough. */
    if (MonType[mon_id].intelligence <= 10-UU.challenge) {
        return false;
    }

    // Buffer holding desirability values used to find a path to the player
    int8_t pathbuf[MAXX][MAXY];

    // Initialize pathbuf
    movesmart_init_pathbuf(pathbuf, mon_id, winleft, winright, wintop, winbot);

    // Fill in the path values
    bool foundit = movesmart_fill_distance(pathbuf, x, y, winleft, winright,
                                           wintop, winbot, distance);
    if (!foundit) { return false; }

    // Attempt to do the move, returning true on success
    return movesmart_do_move(pathbuf, x, y, distance);
}/* movesmart*/


// Move the monster at (x,y) as an unintelligent creature.
static void
movedumb(int8_t x, int8_t y) {

    // Rect. to search for the best position
    int8_t xl = x-1;
    int8_t yl = y-1;
    int8_t xh = x+2;
    int8_t yh = y+2;

    /* for each square compute distance to player */
    int bestx = -1, besty = -1, bestd = INT_MAX;
    for (int8_t xpt = xl; xpt < xh; xpt++) {
        for (int8_t ypt = yl; ypt < yh; ypt++) {

            // Skip if this leaves the map
            if (!inbounds(xpt, ypt)) { continue; }

            uint8_t otype = at(xpt, ypt)->obj.type;

            if (// Not a wall (or the player in the wall).
                (otype != OWALL || (xpt == UU.x && ypt == UU.y))    &&

                // Not the dungeon exit
                (otype != OEXIT)                                    &&

                // no monster already there
                (at(xpt, ypt)->mon.id == 0)                         &&

                // vampires not move towards mirrors
                !(at(x, y)->mon.id == VAMPIRE && otype != OMIRROR)  &&

                // not move towards closed door
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
movemt(int8_t x, int8_t y, int8_t winleft, int8_t winright, int8_t wintop, int8_t winbot,
       int8_t distance) {
    struct Monster mon;

    mon = at(x, y)->mon;

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
    struct Object dob = at(xdest, ydest)->obj;
    struct Monster mon = at(xdest, ydest)->mon;

    if (!ismon(mon)) return;
    if (dob.type != OPIT && dob.type != OTRAPDOOR) return;
    if (avoidspits(mon)) return;

    at(xdest, ydest)->mon = NULL_MON;  /* fell in a pit or trapdoor */
}/* checkpit*/


// If the creature walked onto a sphere of annihilation, handle that.
static void
checksphere(int xdest, int ydest) {
    struct Object dob = at(xdest, ydest)->obj;
    struct Monster mon = at(xdest, ydest)->mon;
    bool tos;

    if (!ismon(mon)) return;
    if (dob.type != OANNIHILATION) return;

    tos = has_a(OSPHTALISMAN);

    /* demons dispel spheres */
    if ((mon.id >= DEMONLORD1 && !tos) ||
        (tos && mon.id == DEMONKING && (rnd(10) > 7))) {
        say("The %s dispels the sphere!\n", MonType[mon.id].name);
        rmsphere(&UU.spherelist, xdest,ydest);    /* delete the sphere */
        return;
    }/* if */

    /* Otherise, poof! */
    at(xdest, ydest)->mon = NULL_MON;
}/* checksphere*/


/* If the monster is a lemming, there's a chance that it reproduced on
 * move. */
static void
checklemming(int xdest, int ydest, int xsrc, int ysrc) {
    struct Monster mon = at(xdest, ydest)->mon;

    if (mon.id != LEMMING) { return; }

    // 2% chance moving creates a new lemming during the annoying
    // phase, then 0.5%.  (Fun fact: this used to be 10%; there are
    // sweary comments about it in the ularn source code about it.)
    int threshold = annoying_lemmings() ? 200 : 50;
    if (mon.id == LEMMING && rnd(10000) <= threshold) {
        at(xsrc, ysrc)->mon = mon;
    }/* if */
}/* checklemming*/


/* If the monster is a leprechaun, pocket whatever valuables are here. */
static void
checkleprechaun(int xdest, int ydest) {
    struct Object dob = at(xdest, ydest)->obj;
    struct Monster mon = at(xdest, ydest)->mon;

    if (mon.id != LEPRECHAUN) return;
    if (isshiny(dob) && lev()->numStolen < (2*MAX_STOLEN)/3) {
        add_to_stolen(dob);
        at(xdest, ydest)->obj = NULL_OBJ;
    }/* if */
}/* checkleprechaun*/


 /* if a troll regenerate him */
static void
checktroll(int xdest, int ydest) {
    struct Monster mon = at(xdest, ydest)->mon;

    if (mon.id != TROLL) return;
    if (UU.gtime & 1) return;   /* Only every second turn. */

    if (maxhp(mon) > mon.hitp) {
        mon.hitp++;
        at(xdest, ydest)->mon = mon;
    }/* if */
}/* checktroll*/


/* Check for dart- or arrow-trap. */
static void
checkpointy(int xdest, int ydest) {
    struct Object dob = at(xdest, ydest)->obj;
    struct Monster mon = at(xdest, ydest)->mon;
    const char *who, *what;

    if (!ismon(mon)) return;
    if (dob.type != OTRAPARROW && dob.type != ODARTRAP) return;

    if (dob.type == OTRAPARROW) {
        who = "An arrow";
        mon.hitp -= rnd(10)+getlevel();
    } else {
        who = "A dart";
        mon.hitp -= rnd(6);
    }/* if .. else*/

    struct Monster new_mon = NULL_MON;
    if (mon.hitp <= 0) {
        what = "%s hits and kills the %s.\n";
    } else {
        new_mon = mon;
        what = "%s hits the %s.\n";
    }/* if .. else*/

    at(xdest, ydest)->mon = new_mon;

    /* Print the message. */
    if (!UU.blindCount) {
        say(what, who, monname_mon(mon));
    }/* if */
}/* checkpointy*/


/* Check for elevator or teleport. */
static void
checkzapper(int xdest, int ydest) {
    struct Object dob = at(xdest, ydest)->obj;
    struct Monster mon = at(xdest, ydest)->mon;
    const char *what;

    if (!ismon(mon) || isdemon(mon)) return;

    if (dob.type == OTELEPORTER) {
        what = "The %s gets teleported.\n";
        teleportmonst(xdest, ydest);
    } else if (dob.type == OELEVATORDOWN || dob.type == OELEVATORUP) {
        what = "The %s is carried away by an elevator!\n";
        at(xdest, ydest)->mon = NULL_MON;
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
mmove(int8_t xsrc, int8_t ysrc, int8_t xdest, int8_t ydest) {

    /* Case 1: moving to player's square attacks him/her. */
    if (xdest == UU.x && ydest == UU.y) {
        hitplayer(xsrc, ysrc);
        at(xsrc, ysrc)->mon.moved = 1;
        return;
    }

    /* Actually move the monster. */
    at(xdest, ydest)->mon = at(xsrc, ysrc)->mon;
    at(xdest, ydest)->mon.moved = 1;
    at(xdest, ydest)->mon.awake = 1;
    at(xsrc, ysrc)->mon = NULL_MON;

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

        if ((at(destx, desty)->obj.type == 0)
            && (at(destx, desty)->mon.id == 0)
            && ((UU.x != destx) || (UU.y != desty))) {
            break;
        }/* if */
    }/* for */

    if (!trys) {
        return;
    }/* if */

    ASSERT (at(destx, desty)->obj.type == 0 &&
            at(destx, desty)->mon.id == 0);

    monid = at(srcx, srcy)->mon.id;

    at(destx, desty)->mon = at(srcx, srcy)->mon;
    at(destx, desty)->mon.hitp = mon_hp(monid);
    see_at(destx, desty);

    at(srcx, srcy)->mon = NULL_MON;

    return;
}/* teleportmonst */
