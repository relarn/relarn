// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "internal_assert.h"
#include "game.h"

#include "display.h"

static void botside(void);
static void drawscreen(void);


void
update_display(bool thorough) {
    if (thorough) {
        drawscreen();
        showstats(&UU, !!GS.wizardMode);
        botside();
    }/* if */

    sync_ui(thorough);
}/* update_display*/


/* Unconditionally update the stats. */
void
update_stats() {
    showstats(&UU, !!GS.wizardMode);
    botside();
    sync_ui(false);
}/* update_stats*/



/* Return the map symbol of the monster all the mimics resemble right
 * now. */
static char
mimicmonst() {
    static uint8_t mimicmonst = MIMIC;
    static long changed_time = -1;

    /* If this is the first time this function has been called,
     * initialize changed_time.  Keeps mimics looking like mimics for
     * the first 10 mobuls.  (I'm not sure if that's a good idea, but
     * that's what the original code did. --CR)*/
    if (changed_time < 0) {
        changed_time = UU.gtime;
    }/* if */

    if (UU.gtime > changed_time + 10) {
        do {
            mimicmonst = rnd(MAXCREATURE);
        } while(mimicmonst == INVISIBLESTALKER);
        changed_time = UU.gtime;
    }/* if */

    return MonType[mimicmonst].mapchar;
}/* shuffle_mimic*/


static void
botside() {
    int i;
    static long* bot_data[] = {
        &UU.stealth,
        &UU.undeadpro,
        &UU.spiritpro,
        &UU.charmcount,
        &UU.timestop,
        &UU.holdmonst,
        &UU.giantstr,
        &UU.fireresistance,
        &UU.dexCount,
        &UU.strcount,
        &UU.scaremonst,
        &UU.hasteSelf,
        &UU.cancellation,
        &UU.invisibility,
        &UU.altpro,
        &UU.protectionTime,
        &UU.wtw,

        NULL,
    };

    for (i = 0; i < IND_MAX ; i++) {
        long *stat = bot_data[i];

        ASSERT(!!stat);

        setindicator((enum INDICATOR)i, !! *stat);
    }/* for */
}/* botside*/


/* Draw (well, schedule for drawing) the map item at position x,y on
 * the map. */
static void
drawcell(int x, int y) {
    uint8_t mon_id, obj_id;
    struct MapSquare here = Map[x][y];

    /* Display the player if they're here. */
    if (x == UU.x && y == UU.y) {
        char player = UU.blindCount > 0 ? ' ' : '@';
        mapdraw(x, y, player, UU.invisibility ? MF_PLAYER_INV:MF_PLAYER);
        return;
    }/* if */

    /* Display empty space if unknown. */
    if (!here.know) {
        mapdraw(x, y, Types[ONONE].symbol, MF_DEFAULT);
        return;
    }/* if */

    mon_id = here.mon.id;
    obj_id = here.obj.type;

    /* Display the object (or ONONE) if no (visible) monster here. */
    if (!mon_id || cantsee(here.mon)) {
        char symbol = Types[obj_id].symbol;
        enum MAPFLAGS flag =
            (obj_id == OWALL || symbol == ' ')
            ? MF_DEFAULT : MF_OBJ;  /* Invert everything except walls,
                                       space, or things that look like
                                       them. */
        mapdraw(x, y, symbol, flag);
        return;
    }/* if */
                
    /* If it's a mimic, it's special case. */
    if (mon_id == MIMIC) {
        mapdraw(x, y, mimicmonst(), MF_DEFAULT);
        return;
    }/* if */

    // XXX WRONG! This should not draw the monster unless it's within
    // the radius of the player's view.
    
    /* Otherwise, it's the monster. */
    mapdraw(x, y, MonType[mon_id].mapchar, MF_DEFAULT);
}/* drawcell*/


/*
**  drawscreen()
**
**  redraw the whole screen as the player knows it
*/
static void
drawscreen() {
    int x, y;

    /* Return unless the map has been initialized. */
    if (getlevel() < 0) return;
    
    for (y=0; y<MAXY; y++) {
        for (x=0; x<MAXX; x++) {
            drawcell(x, y);
        }/* for */
    }/* for */

    sync_ui(false);
}/* drawscreen*/


/* Reveal the region surrounding the player according to their vision
 * level (i.e. blindness and awareness). */
void
showplayerarea() {
    int radius = 1;

    if (UU.blindCount) {
        radius = 0;
    } else if (UU.awareness) {
        radius = 3;
    }/* if */

    showcellat(UU.x, UU.y, radius, radius);
}/* showplayerarea*/

/* Reveal and display everything within normal view of x,y to a range
 * of radius+1 (i.e. radius squares from x,y). */
void
showcellat(int x, int y, int xradius, int yradius) {
    int minx, maxx, miny, maxy;

    minx = x - xradius;
    maxx = x + xradius;
    miny = y - yradius;
    maxy = y + yradius;

    /* Adjust for blindness or awareness. */

    VXY(minx, miny);
    VXY(maxx, maxy);

    for (int yp = miny; yp <= maxy; yp++) {
        for (int xp = minx; xp <= maxx; xp++) {
            Map[xp][yp].know = true;
        }/* for */
    }/* for */

    drawscreen();
}/* showcellat*/

/* Show the cell at x,y and mark it as known. */
void
show1cell(int x, int y) {
    if (UU.blindCount)
        return; /* see nothing if blind     */

    Map[x][y].know = true;
    drawcell(x, y);
    sync_ui(false);
}/* show1cell*/


