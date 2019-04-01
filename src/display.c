// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Note:
//
// In general, modern terminals/consoles/ttys are fast and efficient
// enough that there's no point in optimizing for them.  However, it
// occurred to me recently that some people are going to be logging in
// over an ssh connection, which is both slower and may cost someone
// money to do.  As such, I've started making some effort to not waste
// bandwidth when it doesn't make the code too complex.
//


#include "internal_assert.h"
#include "game.h"

#include "display.h"

#include "fov/fov.h"

static void drawscreen(bool);

// Dirty flags for screen update.
static bool FovChanged = true;
static bool MapChanged = true;

// The grid of squares visible right now
static bool VisibleMap[MAXX][MAXY];

enum UpdateMode {
    // Note: order is significant
    UM_NOMAP, UM_FOV, UM_FULLMAP, UM_REDRAW_ALL
};

struct FovRect {
    int left, right, top, bottom;
    int radius;
};


static void
perform_update (enum UpdateMode mode) {
    bool force = (mode == UM_REDRAW_ALL);

    showstats(&UU, !!GS.wizardMode, force);

    show_indicators(&UU, force);

    if (mode > UM_NOMAP) {
        drawscreen(mode >= UM_FULLMAP);
    }// if

    sync_ui(force);}
// perform_update


// Update the display if it's dirty
void
update_display() {
    enum UpdateMode mode =  MapChanged   ? UM_FULLMAP    :
                            FovChanged   ? UM_FOV        :
                                           UM_NOMAP;

    // monster_detection enforces a full redraw
    mode = UU.monster_detection > 0 ? UM_FULLMAP : mode;

    perform_update(mode);

    FovChanged = MapChanged = false;
}/* update_display*/

// Redraw the entire display, ignoring the dirty flag.
void
redraw() {
    perform_update(UM_REDRAW_ALL);
    FovChanged = MapChanged = false;
}// redraw

void see_and_update_at(int x, int y) {
    see_at(x, y);

    if (player_sees(x,y)) {
        FovChanged = true;
    } else {
        MapChanged = true;
    }// if .. else
}// see_and_update_at

void
force_full_update() {
    MapChanged = true;
}// force_full_update

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


/* Draw (well, schedule for drawing) the map item at position x,y on
 * the map. */
static void
drawcell(int x, int y) {
    struct MapSquare here = Map[x][y];

    /* Display the player if they're here. */
    if (x == UU.x && y == UU.y) {
        char player = UU.blindCount > 0 ? ' ' : '@';
        mapdraw(x, y, player, UU.invisibility ? MF_PLAYER_INV : MF_PLAYER);
        return;
    }/* if */

    bool in_fov = player_sees(x, y);

    /* If monster detection is enabled (i.e. the player drank the
     * relevant potion), we display all monsters on the map,
     * regardless of whether it's in the FoV or a known location.  In
     * addition, this overrides monster abilities. */
    if (UU.monster_detection > 0 && ismon(here.mon)) {
        mapdraw(x, y, monchar(here.mon.id), in_fov ? MF_FOV : MF_DEFAULT);
        return;
    }// if

    /* Display empty space if unknown. */
    if (!known(here)) {
        mapdraw(x, y, Types[ONONE].symbol, MF_NOTSEEN);
        return;
    }/* if */

    // If there's a monster here and visible and we're within FoV,
    // show that.
    if (in_fov && ismon(here.mon) && !cantsee(here.mon)) {
        char mon_char = MonType[here.mon.id].mapchar;

        /* If it's a mimic, it's special case. */
        if (here.mon.id == MIMIC) {
            mon_char = mimicmonst();
        }/* if */

        mapdraw(x, y, mon_char, MF_FOV);
        return;
    }// if

    // Otherwise, display the recalled object at this location.  (We
    // assume that see_and_update_fov() has previously been called to
    // update the recollection of stuff within FoV, so this will still
    // be accurate.)
    char symbol = Types[here.recalled.type].symbol;

    /* Invert everything except walls, space, or things that look like
     * them. */
    enum MAPFLAGS flag =
        (here.recalled.type != OWALL && symbol != ' ') ? MF_OBJ :
        in_fov                                         ? MF_FOV : MF_DEFAULT;
    mapdraw(x, y, symbol, flag);
}/* drawcell*/


static struct FovRect
get_vis_rect() {
    int radius = 2;

    if (UU.blindCount) {
        radius = 0;
    } else if (UU.enlightenment.time) {
        radius = UU.enlightenment.radius;
    } else if (UU.awareness) {
        radius = 5;
    }/* if */

    struct FovRect result;
    result.left    = max(0,        UU.x - radius);
    result.right   = min(MAXX - 1, UU.x + radius);
    result.top     = max(0,        UU.y - radius);
    result.bottom  = min(MAXY - 1, UU.y + radius);
    result.radius  = radius;

    return result;
}// get_vis_rect


// Return a copy of fov that has been expanded by 1 in all directions
// but trimmed to stay within the map.  We use this to ensure that
// during normal movement, the area changed by the previous move will
// be redrawn.
static struct FovRect
expand_vis_rect(struct FovRect fov) {
    // Expand by 1 so that we redraw the cells now out of sight
    fov.left -= 1;
    fov.right += 1;
    fov.top -= 1;
    fov.bottom += 1;
    fov.radius += 1;

    VXY(fov.left, fov.top);
    VXY(fov.right, fov.bottom);

    // And bump by 1 so that we include the outermost row and column
    fov.right += 1;
    fov.bottom += 1;

    return fov;
}

// Redraw the screen.  If 'all' is false, skip some of the area
// outside the field-of-view rectangle.  Correctly handles the case
// where FoV shrinks but not long-distance movement
// (e.g. teleportation); for that case, the caller will need to redraw
// everything.
static void
drawscreen(bool all) {
    static struct FovRect ofov = {0,0,0,0,0};   // Previous FovRect

    /* Return unless the map has been initialized. */
    //if (getlevel() < 0) return;
    ASSERT(getlevel() >= 0);

    // Figure out the subsection of the map to draw.
    struct FovRect fov =
        all ? (struct FovRect){0, MAXX, 0, MAXY, MAXX} : get_vis_rect();

    // If the FoV has shrunk, we need to draw the previous size at
    // least once more to update the areas that are no longer visible.
    // Otherwise, we stash the previous FoV for next time.
    if (fov.radius < ofov.radius) {
        struct FovRect ofov_copy = ofov;
        ofov = fov;
        fov = ofov_copy;
    } else {
        ofov = fov;
    }// if .. else

    // Draw the cells
    fov = expand_vis_rect(fov);     // Expand to update the adjacent cells
    for (int y = fov.top; y < fov.bottom; y++) {
        for (int x = fov.left; x < fov.right; x++) {
            drawcell(x, y);
        }/* for */
    }/* for */
}// drawscreen


// Callback to mark this position as visible
static
void fov_apply(void *map, int x, int y, int dx, int dy, void *src) {
    if (x < 0 || x >= MAXX || y < 0 || y >= MAXY) { return; }
    VisibleMap[x][y] = true;
}// fov_apply

// Callback to return true if the object at x,y is opaque (or if its
// off the map).
static bool
fov_opaque_test(void *map, int x, int y) {
    if (x < 0 || x >= MAXX || y < 0 || y >= MAXY) { return true; }
    if (UU.enlightenment.time > 0) { return false; }
    return isopaque(Map[x][y].obj);
}// fov_opaque_test


static void
update_visible_map(struct FovRect fov_rect) {
    static bool fovStructInitialized = false;
    static fov_settings_type fovSettings;

    // Initialize fovSettings on the first call
    if (!fovStructInitialized) {
        fov_settings_init(&fovSettings);
        fov_settings_set_opacity_test_function(&fovSettings, fov_opaque_test);
        fov_settings_set_apply_lighting_function(&fovSettings, fov_apply);

        fovStructInitialized = true;
    }// if

    // Clear the visible map
    memset(VisibleMap, 0, sizeof(VisibleMap));

    // fov_circle() sometimes(?) skips the source cells so we set it
    // here since the square you occupy will always be mapped.
    VisibleMap[UU.x][UU.y] = true;

    // And then we skip the calculation if blind.
    if (fov_rect.radius == 0) {
        return;
    }// if

    // Compute the visibility
    fov_circle(&fovSettings, NULL, NULL, UU.x, UU.y, fov_rect.radius);
}// update_visible_map



// Reveal (mark known and dirty) everything within view
void
see_and_update_fov() {
    // Compute the outermost visibility rectangle
    struct FovRect fov_rect = get_vis_rect();

    // Compute line-of-sight and write it to VisibleMap
    update_visible_map(fov_rect);

    // Update the map to include those points in the current FoV:
    bool blindwtw = UU.blindCount > 0 && UU.wtw > 0;
    for (int y = fov_rect.top; y <= fov_rect.bottom; y++) {
        for (int x = fov_rect.left; x <= fov_rect.right; x++) {

            // Skip it if the spot isn't lit
            if (!VisibleMap[x][y]) { continue; }

            // If you're blind and can walk through walls, you
            // can't tell the difference between a wall and open
            // space.
            if (
                blindwtw &&
                (Map[x][y].obj.type == OWALL || Map[x][y].obj.type == ONONE) &&
                !known_at(x, y)
                ) {
                continue;
            }

            see_at(x, y);
        }// for
    }// for

    // And mark the Fov rectangle dirty
    FovChanged = true;
}// see_and_update_fov

bool
player_sees(int x, int y) {
//    struct FovRect fov = get_vis_rect();
//    return fov.left <= x && x <= fov.right && fov.top <= y && y <= fov.bottom;
    return VisibleMap[x][y];
}// player_sees


void
flash_at(int x, int y, char c, int period) {
    update_display();

    mapdraw(x, y, c, MF_EFFECT);
    sync_ui(false);

    nap(period);
    drawcell(x, y);
    sync_ui(false);
}// flash_at
