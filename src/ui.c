// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "internal_assert.h"
#include "display.h"
#include "settings.h"
#include "version_info.h"
#include "player.h"

#include "ui.h"

#include "curses_extensions.h"

#include <curses.h>
#include <math.h>
#include <time.h>

#define ESC '\033'
#define CTRL_N 14
#define CTRL_P 16
#define CTRL_V 22
#define CTRL_X 24

#define CONSOLE_Y (StatsY + StatsHeight)
#define CONSOLE_H 6

enum ColorPairs {
    CLR_PLAYER = 1,
    CLR_INVIS,
    CLR_EFFECT,
    CLR_UNSEEN,
    CLR_UNSEEN_DARK,
    CLR_FOV,
    CLR_FOV_DARK,
    CLR_FOV_TOWN,

    // Debug colors
    CLR_DBG1, CLR_DBG2, CLR_DBG3, CLR_DBG4,
    CLR_DBG5, CLR_DBG6, CLR_DBG7
};

static bool pick_backend(struct PickList *pl, const char *headings[],
                         int hcount, bool *selections, bool multi);
static void backspace(void);

static struct TextBuffer *ConsoleBuffer = NULL;
static int ScrollbackPos = 0;

static WINDOW *ConsoleWin = NULL;
static WINDOW *IndWin = NULL;
static WINDOW *MapWin = NULL;
static WINDOW *StatsWin = NULL;

static const int MapWidth = MAXX;
static const int MapHeight = MAXY;

static const int IndWidth = SCREEN_W - MAXX;
static const int IndHeight = MAXY;

static const short StatsY = MAXY;
static const short StatsHeight = 2;


// Normalize the keycode corresponding to the ENTER key.  That is,
// Replace any of the three(?) keycodes that can be interpreted as a
// RETURN or ENTER as '\n'.
static int
norm_return(int key) {
    if (key == KEY_ENTER || key == '\n' || key == '\r') { return '\n'; }
    return key;
}


// Test if 'key' should be interpreted as backspace.
static bool
is_backspace(int key) {
    return key == KEY_BACKSPACE || key == KEY_DC || key == 127 || key == 8;
}

// Rotating colors as a debugging aid
static void
debugCycleBackground (WINDOW *win) {
    static int curr = 0;
    static int DBG_COLORS[] = {
        CLR_DBG1, CLR_DBG2, CLR_DBG3, CLR_DBG4,
        CLR_DBG5, CLR_DBG6, CLR_DBG7,
    };

    // Do nothing unless debugging is enabled
    if (!GameSettings.drawDebugging) { return; }

    curr = rund(sizeof(DBG_COLORS)/sizeof(chtype));
    wattrset(win, COLOR_PAIR(DBG_COLORS[curr]));
}// getDbgClr



static void
set_clr(short id, struct Color clr) {
    if (!clr.isSet) { return; }

    short r = (short)round( (double)(clr.r) * 1000.0/ 0xFF );
    short g = (short)round( (double)(clr.g) * 1000.0/ 0xFF );
    short b = (short)round( (double)(clr.b) * 1000.0/ 0xFF );

    init_color(id, r, g, b);
}// set_clr

static void
set_requested_colors() {
    if (!can_change_color()) { return; }

    set_clr(COLOR_BLACK,    GameSettings.black);
    set_clr(COLOR_RED,      GameSettings.red);
    set_clr(COLOR_GREEN,    GameSettings.green);
    set_clr(COLOR_YELLOW,   GameSettings.yellow);
    set_clr(COLOR_BLUE,     GameSettings.blue);
    set_clr(COLOR_MAGENTA,  GameSettings.magenta);
    set_clr(COLOR_CYAN,     GameSettings.cyan);
    set_clr(COLOR_WHITE,    GameSettings.white);
}// set_requested_colors


void
init_ui() {
    ASSERT(!ConsoleWin && !IndWin);

    // Set up stuff specific to the curses implemention we're using
    setup_curses_preconditions();

    initscr();
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, true);

    // Set up more stuff specific to the curses implemention we're
    // using that needs to be done *after* initscr() is called.
    setup_curses_extensions();

    if (has_colors()) {
        start_color();

        use_default_colors();

        set_requested_colors();

        init_pair(CLR_PLAYER, COLOR_RED, -1);
        init_pair(CLR_INVIS, COLOR_CYAN, -1);
        init_pair(CLR_EFFECT, COLOR_CYAN, -1);

        // Colour(s) for undiscovered map squares
        setup_unseen_area_colors(CLR_UNSEEN, CLR_UNSEEN_DARK);

        init_pair(CLR_FOV_DARK, COLOR_BLACK, COLOR_YELLOW);
        init_pair(CLR_FOV, -1, COLOR_YELLOW);
        init_pair(CLR_FOV_TOWN, COLOR_BLACK, COLOR_GREEN);

        // These are used for display debugging
        init_pair(CLR_DBG1, -1, COLOR_RED);
        init_pair(CLR_DBG2, -1, COLOR_GREEN);
        init_pair(CLR_DBG3, -1, COLOR_YELLOW);
        init_pair(CLR_DBG4, -1, COLOR_BLUE);
        init_pair(CLR_DBG5, -1, COLOR_MAGENTA);
        init_pair(CLR_DBG6, -1, COLOR_CYAN);
        init_pair(CLR_DBG7, -1, COLOR_WHITE);
    }/* if */

    ConsoleWin = newwin(CONSOLE_H, SCREEN_W, CONSOLE_Y, 0);
    scrollok(ConsoleWin, true);
    idlok(ConsoleWin, true);

    StatsWin = newwin(StatsHeight, SCREEN_W, StatsY, 0);
    IndWin = newwin(IndHeight, IndWidth, 0, MapWidth);
    MapWin = newwin(MapHeight, MapWidth, 0, 0);

    // Sanity check
    ENSURE_MSG(ConsoleWin && StatsWin && IndWin && MapWin,
               "Error creating curses window.");

    keypad(MapWin, true);
}/* init_ui*/

void
teardown_ui() {

    // Tolerate calling this multiple times.
    if (!ConsoleWin || !IndWidth || !MapWin || !StatsWin) {
        return;
    }// if

    delwin(ConsoleWin);
    delwin(IndWin);
    delwin(MapWin);
    delwin(StatsWin);

    ConsoleWin = NULL;
    IndWin = NULL;
    MapWin = NULL;
    StatsWin = NULL;

    endwin();
}/* teardown_ui*/

/* If the UI delays displaying results, it should update now.  If
 * 'force' is true, redraw everything. */
void
sync_ui(bool force) {

    if (force) {
        redrawwin(ConsoleWin);
        redrawwin(IndWin);
        redrawwin(MapWin);
        redrawwin(StatsWin);
    }

    /* This is equivalent to, but more efficient than calling
     * wrefresh() on each window. */
    wnoutrefresh(ConsoleWin);
    wnoutrefresh(IndWin);
    wnoutrefresh(MapWin);
    wnoutrefresh(StatsWin);

    doupdate();
}/* sync_ui*/


// Write out a line of text to the StatsWin window.  If '^' is
// encountered, modify the text (either bold or reverse) until '|' is
// found.  This is used to hilight changed stats on the display.
static void
write_stats_line(const char *line) {
    chtype mod = 0;
    for (int n = 0; line[n]; n++) {
        chtype c = line[n];
        if (c == '^') {
            mod = GameSettings.hilightReverse ? A_REVERSE : A_BOLD;
        } else if (c == '|') {
            mod = 0;
        } else {
            waddch(StatsWin, c | mod);
        }
    }
}


struct StatDisp {
    long value;
    time_t age;
};

// Return '^' or '|' (hilight or don't highlight) depending on whether
// the item in `stati[*ndx]` is different from `value` or has changed
// in the last few seconds.  (This is used to highlight changed
// stats.)
static char
hl(struct StatDisp stati[], size_t *ndx, long value, bool disable) {
    time_t now = time(NULL);

    if (value != stati[*ndx].value) {
        stati[*ndx].age = GameSettings.hilightTime + now;
        stati[*ndx].value = value;
    }

    if (disable) {
        stati[*ndx].age = 0;
    }

    char result =  stati[*ndx].age > now ? '^' : '|';
    ++*ndx;
    return result;
}// hl

// Display the stats in StatsWin, temporarily highlighting those that
// changed.
void
showstats(bool iswiz, bool force) {
    bool hidefloor = !lev()->known;

    static struct StatDisp stati[20];  // oversize
    size_t n = 0;
    static bool disable = true;

    // Macro to produce a pair of arguments: the highlight character
    // for the stat to determine whether it should be highlighted
    // followed by the actual value cast to long.
#define STATPAIR(s) hl(stati, &n, s, disable), (long)(s)

    char line1[120];
    snprintf(line1, sizeof(line1),
             "Spells:%3ld(%c%2ld|) AC:%c%-3ld| WC:%c%-3ld| LV:%c%-2ld| %s:%-4ld"
             " Exp: %-9ld %s",
             (long)UU.spells,

             STATPAIR(UU.spellmax),
             STATPAIR(stat_val(&UU.defense)),
             STATPAIR(weaponclass()),
             STATPAIR(UU.level),

             iswiz ? "Trns": "Time",
             iswiz ? (long)UU.gtime : (long)(UU.gtime / MOBUL),
             (long)UU.experience, levelDesc(UU.level));

    char buf[20], line2[120];
    snprintf(buf, sizeof(buf), "%d (%c%ld|)",
             (int)UU.hp,
             STATPAIR(UU.hpmax));

    snprintf(line2, sizeof(line2),
             "HP: %11s STR=%c%-2ld| INT=%c%-2ld| WIS=%c%-2ld| CON=%c%-2ld| "
             "DEX=%c%-2ld| CHA=%c%-2ld| LV:%2s%s Gold: %-8ld",
             buf,

             STATPAIR(stat_val(&UU.strength)),
             STATPAIR(stat_val(&UU.intelligence)),
             STATPAIR(stat_val(&UU.wisdom)),
             STATPAIR(stat_val(&UU.constitution)),
             STATPAIR(stat_val(&UU.dexterity)),
             STATPAIR(stat_val(&UU.charisma)),

             hidefloor ? " ?" : getlevelname(),
             iswiz ? " W" : "  ",
             (long)UU.gold);

#undef STATPAIR

    ASSERT(n < sizeof(stati) / sizeof(stati[0]));

    debugCycleBackground(StatsWin);

    wmove(StatsWin, 0, 0);
    write_stats_line(line1);

    wmove(StatsWin, 1, 0);
    write_stats_line(line2);

    disable = false;
}/* showstats*/


// Wait for and fetch a character from the input associated with the
// map window.  This gets used by the main event loop.  Also
// translates a few key sequences.
char
map_getch() {
    int key = wgetch(MapWin);

    // Translate arrow keys.
    switch(key) {
    case KEY_UP:    return 'k';
    case KEY_DOWN:  return 'j';
    case KEY_LEFT:  return 'h';
    case KEY_RIGHT: return 'l';
    }

    return (char)key;
}// map_getch


struct MapAttrs {
    chtype effect;          // Visual effects (e.g. magic missile)
    chtype obj;             // Objects on the map
    chtype player;          // The player
    chtype player_inv;      // The player when invisible
    chtype notseen;         // An undiscovered location
    chtype fov;             // The current field of view
    chtype fov_town;        // The town looks much nicer
};

// Attributes for light-background screens
static struct MapAttrs
brightAttrs() {
    struct MapAttrs attr;

    memset(&attr, 0, sizeof(attr));

    attr.effect =        A_REVERSE|A_DIM|COLOR_PAIR(CLR_EFFECT);
    attr.obj =           A_REVERSE;
    attr.player =        A_REVERSE|COLOR_PAIR(CLR_PLAYER);
    attr.player_inv =    A_REVERSE|COLOR_PAIR(CLR_INVIS);

    attr.notseen = GameSettings.showUnrevealed
        ? COLOR_PAIR(CLR_UNSEEN)
        : 0;

    attr.fov = GameSettings.showFoV
        ? A_BOLD|COLOR_PAIR(CLR_FOV)
        : 0;

    attr.fov_town = GameSettings.showFoV
        ? A_BOLD|COLOR_PAIR(CLR_FOV_TOWN)
        : 0;

    return attr;
} // brightAttrs

static struct MapAttrs
darkAttrs() {
    struct MapAttrs attr;

    memset(&attr, 0, sizeof(attr));

    attr.effect =        A_REVERSE|A_DIM|COLOR_PAIR(CLR_EFFECT);
    attr.obj =           A_REVERSE;
    attr.player =        A_REVERSE|COLOR_PAIR(CLR_PLAYER);
    attr.player_inv =    A_REVERSE|COLOR_PAIR(CLR_INVIS);

    attr.notseen = GameSettings.showUnrevealed
        ? COLOR_PAIR(CLR_UNSEEN_DARK)
        : 0;

    attr.fov = GameSettings.showFoV
        ? COLOR_PAIR(CLR_FOV_DARK)
        : 0;

    attr.fov_town = GameSettings.showFoV
        ? A_BOLD|COLOR_PAIR(CLR_FOV_TOWN)
        : 0;

    return attr;
} // darkAttrs



/* Display one character on the map. */
void
mapdraw(int x, int y, char sym, enum MAPFLAGS flags, bool inFoV, bool isTown) {
    ASSERT(x >= 0 && y >= 0 && x < MapWidth && y < MapHeight);

    // Set of modifiers for displaying map squares; depends on screen
    // settings.
    static struct MapAttrs attrs;
    static bool mods_initialized = false;
    if (!mods_initialized) {
        attrs = GameSettings.darkScreen ? darkAttrs() : brightAttrs();
    }

    // FOV flags if requested
    const chtype fov =
        !inFoV              ? 0 :
        isTown              ? attrs.fov_town :
                              attrs.fov;
    chtype symbol = sym;

    switch(flags) {
    case MFL_EFFECT:     symbol |= attrs.effect;         break;
    case MFL_OBJ:        symbol |= fov|attrs.obj;        break;
    case MFL_PLAYER:     symbol |= attrs.player;         break;
    case MFL_PLAYER_INV: symbol |= attrs.player_inv;     break;
    case MFL_NOTSEEN:    symbol |= attrs.notseen;        break;

    case MFL_DEFAULT:    symbol |= fov;                  break;
    }/* switch*/

    // Display debug functionality: randomize the drawing colour when
    // we reach the player position.
    if (x == UU.x && y == UU.y) {
        debugCycleBackground(MapWin);
    }

    mvwaddch(MapWin, y, x, symbol);
}/* mapdraw*/


/* Write 'text' at the current position.  If a segment is bracketted
 * with '_', it is underlined and sections bracketted with '|' are
 * reversed. */
static void
addfmt (WINDOW *win, const char *text) {
    bool isBold = false;
    bool isUnderline = false;

    bool escape = false;
    for (int n = 0; text[n]; n++) {
        int letter = text[n];

        if (!escape && letter == '\\') {
            escape = true;
            continue;
        }

        if (!escape && letter == '|') {
            isBold = !isBold;
            continue;
        }/* if */

        if (!escape && letter == '_') {
            isUnderline = !isUnderline;
            continue;
        }/* if */

        if (!escape && letter == '/') {
            wrefresh(win);
            nap(1000);
            continue;
        }/* if */

        letter = letter | (isBold ? A_BOLD : 0) |
            (isUnderline ? A_UNDERLINE : 0);

        waddch(win, letter);

        escape = false;
    }/* for */

    wrefresh(win);
}/* addfmt */



/* Given a TextBuffer (basically an array of pointers to strings),
 * display them one page at a time.  Pages are separated by form feeds
 * ('\f') with one line per string.
 *
 * The final entry must be NULL.
 *
 * Individual lines should not be more than 79 characters long.
 *
 * All whitespace characters are replaced with simple spaces.
 *
 * Text between pipes ('|') are displayed standout and text between
 * underscores ('_') are displayed underlined.  '/' pauses for 1
 * second (after forcing a refresh to display what came before) for
 * the dramatic pause.  The closing character must be on the same
 * line--end of line resets this.
 *
 * If 'prompt' is true, prompts the user for confirmation and returns
 * true if the user confirmed and false otherwise.  The result is
 * undefined if 'prompt' is false.
 */
bool
showpages_prompt(struct TextBuffer *tb, bool prompt) {
    int currline = 0;

    WINDOW *win = newwin(0, 0, 0, 0);   // Full-screen window

    char *cont, *done;
    if (prompt) {
        cont = "    ---- |y| or |return| for yes, |n| or |escape| for no;"
               " |space| for more ----";
        done = "    ---- |y| or |return| for yes, |n| or |escape| for no ----";
    } else {
        cont = "    ---- Press |return| or |escape| to exit, "
               "|space| for more ---- ";
        done = "    ---- Press |return| or |escape| to exit ---- ";
    }

    bool confirm = false;
    for (;;) {
        int pos = 0;

        clear();
        while (currline < tb->num_lines &&
               *(tb->text[currline]) != '\f' &&
               pos < SCREEN_H) {
            wmove (win, pos, 0);
            addfmt(win, tb->text[currline]);
            ++pos;
            ++currline;
        }/* while */

        /* If we're at a \f line, advance past it. */
        if (currline < tb->num_lines && *(tb->text[currline]) == '\f') {
            ++currline;
        }/* if */

        wmove(win, SCREEN_H - 1, 0);
        addfmt(win, currline < tb->num_lines ? cont : done);

        wrefresh(win);

        // Loop until the user provides a meaningful keystroke
        for (;;) {
            int i;

            i = norm_return( wgetch(win) );

            // If space, advance to next page
            if (i == ' ' && currline < tb->num_lines) { break; }

            // If just displaying, look for an exit
            if (!prompt) {
                if (i == '\n' || i == ESC) { goto finished; }
                continue;
            }// if

            // Return true or false, depending confirmation.
            if (i == 'y' || i == '\n') {
                confirm = true;
                goto finished;
            }

            if (i == 'n' || i == ESC) { goto finished; }
        }/* for */

        wclear(win);
    }/* for */

finished:
    delwin(win);
    sync_ui(true);

    return confirm;
}// showpages_prompt


// Display a text buffer.  See showpages_prompt().
void
showpages(struct TextBuffer *tb) {
    showpages_prompt(tb, false);
}


/* Get the user to select an item from a list.  Items are entries in
 * 'items', separated by newlines, each containing (at least) one
 * parenthesized letter option (e.g. "... (a)ccept ...").  Returns 0
 * if cancelled or the letter corresponding to the item ('a' for
 * first, 'b' for second, etc.)  Only the first parentisized letter is
 * used.  Letters should be unique.
 */
char
menu(const char *heading, const char* items) {
    int id = 0;
    int i;
    struct PickList *pl;

    char **itemLines;
    int num_itemLines;

    itemLines = splitstring(items, &num_itemLines);

    pl = pl_malloc();
    pl_setflags(pl, PLF_HIDELETTER);

    for (i = 0; i < num_itemLines; i++) {
        const char *curr = itemLines[i];
        char letter = 0;
        int n;

        /* Empty strings are blank lines. */
        if (!*curr) {
            pl_add(pl, 0, 0, "");
            continue;
        }/* if */

        ASSERT(strlen(curr) > 3);

        for (n = 2; curr[n]; n++) {
            if(curr[n] == ')' && curr[n-2] == '(') {
                letter = curr[n-1];
                break;
            }/* if*/
        }/* for */

        ASSERT(letter);

        pl_add(pl, letter, letter, curr);
    }/* for */

    pick_item(pl, heading, &id);

    pl_free(pl);
    free(itemLines[0]);
    free(itemLines);

    return (char)id;
}/* menu*/


static bool *
mk_sel_vec(struct PickList *pl) {
    bool *result = xcalloc(pl_count(pl), sizeof(bool));
    return result;
}// mk_sel_vec

static int*
find_sel_ids(struct PickList *pl, bool *selected, int *num_ids) {
    int *ids = xcalloc(pl_count(pl), sizeof(int));

    int count = 0;
    for (int n = 0; n < pl_count(pl); n++) {
        if (selected[n]) {
            ids[count++] = pl->items[n].id;
        }// if
    }// for

    *num_ids = count;
    return ids;
}// find_sel_indexes


// Let the user select multiple items from PickList `pl`.  Returns the
// number of items selected and a list of indexes stored in *ids
// (which is NULL if no items were selected.)  If not NULL, the
// pointer returned in *ids must be freed.
//
// If 'multi' is false, returns as soon as the first item is selected.
// This kind of defeats the '_multi' aspect but it's useful as a
// backend for pick_item().
int
pick_multi (struct PickList *pl, const char *heading, int **ids, bool multi) {

    ASSERT(ids);

    *ids = NULL;

    int headings_sz = 0;
    char **headings = splitstring(heading, &headings_sz);

    bool *selections = mk_sel_vec(pl);

    bool status = pick_backend(pl, (const char **)headings, headings_sz,
                               selections, multi);

    free(headings[0]);
    free(headings);

    if (!status) {
        free(selections);
        return 0;
    }

    int count;
    *ids = find_sel_ids(pl, selections, &count);
    if (!count) {
        free(*ids);
        *ids = NULL;
    }

    return count;
}// pick_some


/* Display a PickList and set 'id' to the item selected.  Return true
 * if id was set, false if the user cancelled.  If the user cancelled,
 * id is unmodified.  Heading is split into multiple lines on
 * newlines.*/
bool
pick_item (struct PickList *pl, const char *heading, int *id) {

    int *ids;
    int count = pick_multi(pl, heading, &ids, false);
    ASSERT(count <= 1 && count >= 0);
    if (count == 0) { return false; }

    *id = ids[0];
    free(ids);

    return true;
}/* pick_item */


/* Draw a line of a PickList. */
static void
showline(WINDOW *w, int y, int x, struct PickList *pl, bool sel[], int index) {
    char lbuf[5];

    // Format the leading letter
    char letter = pl->items[index].letter;
    if (letter == 0 || (pl->flags & PLF_HIDELETTER)
        || pl->items[index].description[0] == 0) {
        lbuf[0] = 0;
    } else {
        snprintf(lbuf, sizeof(lbuf), "%c.", letter);
    }

    // Format the entire line
    const char *desc = pl->items[index].description;
    const char *mark = sel[index] ? "*" : " ";

    char buffer[SCREEN_W + 1];
    snprintf(buffer, sizeof(buffer), "%2.2s %s %s", lbuf, mark, desc);

    // Print the line
    int width = SCREEN_W - x - 1;
    mvwprintw(w, y, x, "%-*.*s", width, width, buffer);
}/* showline*/


// Create the parent (curses) window for the picker dialog.
static WINDOW *
mk_pick_parent(const char *headings[], int hcount, bool multi) {

    WINDOW *parent = newwin (SCREEN_H, SCREEN_W, 0, 0);
    ENSURE_MSG(parent, "Error initializing Curses window.");

    wclear(parent);
    wrefresh(parent);
    keypad(parent, true);

    /* Print headings */
    for (int n = 0; n < hcount; n++) {
        mvwprintw (parent, n, (SCREEN_W - strlen(headings[n])) / 2,
                   "%s", headings[n]);
    }

    /* Print footer */
    char guide[120];
    snprintf(guide, sizeof(guide),
             "Up:k/CTRL+p/UP Down:j/CTRL+n/DOWN Select:ENTER%s "
             "Quit:ESC/CTRL+x",
             multi ? "/SPC" : ""
        );

    mvwprintw (parent, SCREEN_H - 2, 1, guide);
    mvwprintw (parent, SCREEN_H - 1, 1, "To select an individual item, "
               "type the corresponding key; CTRL+v escapes.");

    return parent;
}// mk_pick_parent


// Create and return the scrolling subwindow used to hold the list of
// items to select.
static WINDOW *
mk_pick_scroller(struct PickList *pl, bool *sel, int height, int y_pos) {
    WINDOW *scroller = newwin (height, SCREEN_W, y_pos, 0);
    ENSURE_MSG(scroller, "Unable to Curses scroller.");

    // Enable scrolling in scroller
    scrollok(scroller, true);
    for (int n = 0; n < pl->num_elems && n < height; n++) {
        if (n == 0) {
            wattron(scroller, A_REVERSE);
        }/* if */
        showline (scroller, n, 3, pl, sel, n);
        if (n == 0) {
            wattroff(scroller, A_REVERSE);
        }/* if */
    }/* for */

    return scroller;
}// mk_pick_scroller


// Hilight the row at *pos_p in the scroller, scrolling if necessary.
static void
redraw_scroller(WINDOW *scroller, struct PickList *pl, bool *selections,
                int scrlHeight, int pos, int *prevPos, int *first_p) {

    // Make a local copy for readability; updated before returning
    int first = *first_p;

    // Scroll if we need to
    if (pos >= first + scrlHeight) {   /* Need to scroll up. */
        wscrl(scroller, 1);
        first++;
    } else if (pos < first) { /* Need to scroll down. */
        wscrl(scroller, -1);
        first--;
    }/* if */

    // Redraw the previous line to undo the reverse video
    showline (scroller, *prevPos - first, 3, pl, selections, *prevPos);

    // Draw the current line reversed.
    wattron(scroller, A_REVERSE);
    showline (scroller, pos - first, 3, pl, selections, pos);
    wattroff(scroller, A_REVERSE);

    // Do the update
    wrefresh(scroller);

    // And store the new position values
    *prevPos = pos;

    *first_p = first;
}// redraw_scroller


// Move the selection to the row indicated by 'key'.  Movement is fast
// but visible so that the user can identify what's happening.
static void
move_selection(int key, WINDOW *scroller, struct PickList *pl,bool *selections,
               int scrlHeight, int *pos, int *prevPos, int *first) {
    int dest = -1;
    for (int i = 0; i < pl->num_elems; i++) {
        if (key == pl->items[i].letter) {
            dest = i;
            break;
        }/* if */
    }/* for */

    if (dest == -1 || dest == *pos) { return; }

    int incr = *pos < dest ? 1 : -1;
    while (*pos != dest) {
        *pos += incr;
        redraw_scroller(scroller, pl, selections, scrlHeight, *pos, prevPos,
                        first);
        napms(7);
    }// while
}// move_selection


// Do the actual work of displaying a picker and letting the user
// select one or more items.  Returns true if the player ever selected
// an item.  Note that for multiple selections, this doesn't mean that
// an item was selected.
static bool
pick_backend(struct PickList *pl, const char *headings[], int hcount,
             bool *selections, bool multi) {
    bool retval = false;
    int pos = 0, prevPos = 0, first = 0;

    /* Ensure hcount isn't too big for the screen.*/
    ASSERT(hcount < SCREEN_H - 15);

    int scrlHeight = (SCREEN_H - 5) - hcount;

    WINDOW *parent = mk_pick_parent(headings, hcount, multi);
    WINDOW *scroller = mk_pick_scroller(pl, selections, scrlHeight, hcount+1);
    wrefresh (parent);
    wrefresh (scroller);

    while (true) {
        bool selected = false;
        int key = norm_return( wgetch(parent) );

        /* Handle literal keys separately. */
        switch(key) {
        case 'j':       key = KEY_DOWN;         break;
        case 'k':       key = KEY_UP;           break;
        case CTRL_V:    key = wgetch(parent);   break;
        }

        // Now do the action
        switch (key) {
        case ESC:
        case CTRL_X:
            goto done;

        case KEY_DOWN:
        case CTRL_N:
            if (pos < pl->num_elems - 1) {
                ++pos;
            }/* if */
            break;


        case KEY_UP:
        case CTRL_P:
            if (pos > 0) {
                --pos;
            }/* if */
            break;

        case ' ':   // Space selects but only in multi mode
            if (!multi) { break; }
            // Fall through...

        case '\n':
            /* Select current item unless it's empty. */
            if (pl->items[pos].description[0]) {
                selections[pos] = !selections[pos];
                selected = true;
            }/* if */
            break;

        default:
            // If the user hit a letter that matches an entry in pl,
            // scroll to it.
            if (isalnum(key)) {
                move_selection(key, scroller, pl, selections, scrlHeight, &pos,
                               &prevPos, &first);
            }/* if */
        }/* switch */


        if (selected) {
            retval = true;
            if (!multi) {
                goto done;
            }
        }// if

        // Hilight the new line, possibly with scrolling.
        redraw_scroller(scroller, pl, selections, scrlHeight, pos, &prevPos,
                        &first);
    }/* while */


done:
    delwin(scroller);
    delwin(parent);

    sync_ui(true);

    return retval;
}/* pick_item */


// Prompt the user for a line of text without using the console
// window.  Returns true if text was selected and is non-empty, false
// if cancelled via ESCAPE.  Text is written to 'result'; if 'result'
// already has text in it, that is used as the initial value.
//
// (Currently, this is only used to let the player enter their
// character name on startup.)
static bool
fullscreen_prompt(const char *prompt, char *result, size_t resultLen) {
    WINDOW *win = newwin(0, 0, 0, 0);   // Full-screen window
    keypad(win, true);

    const int top = 4;
    const int editline_y = top + 2;
    const int left = 4;

    resultLen = min(resultLen, SCREEN_W - left * 2);

    bool accepted = false;
    bool redraw = true;
    size_t end = strlen(result);        // index of trailing null
    while(true) {
        if (redraw) {
            wclear(win);

            wmove(win, top, left);
            addfmt(win, prompt);

            wmove(win, editline_y + 2, left);
            addfmt(win, "|ENTER to accept, ESC to cancel|");

            redraw = false;
        }// if

        wmove(win, editline_y, left);
        waddch(win, '[');
        waddstr(win, result);
        for (int i = end; i < resultLen - 1; i++) {
            waddch(win, '_');
        }
        waddch(win, ']');
        wmove(win, editline_y, left + 1 + end);

        curs_set(2);        // show cursor
        int key = norm_return( wgetch(win) );
        curs_set(0);        // hide it again

        if (key == 12 || key == 18) { // ^R or ^L
            redraw = true;
            continue;
        }

        if (key == ESC)     { break; }
        if (key == '\n')    {
            accepted = true;
            break;
        }

        if (is_backspace(key)) {
            if (end > 0) {
                --end;
                result[end] = 0;
            }
            continue;
        }// if

        if (!isprint(key))  { continue; }

        if (end + 1 >= resultLen) { continue; }

        result[end] = (char)key;
        ++end;
        result[end] = 0;
    }// while


    delwin(win);
    sync_ui(true);

    return accepted && end > 0;
}// fullscreen_prompt



// Let the user select a character class.
static bool
get_cclass(enum CHAR_CLASS *pcc) {
    struct PickList *cclasses = pl_malloc();

    for (int n = 1; n < CC_COUNT; n++) {
        char line[80];

        snprintf(line, sizeof(line), "%s", ccname(n));
        pl_add(cclasses, n, 'a' + n - 1, line);
    }/* for */

    int id = 0;
    bool selected = pick_item(cclasses, "So, what are ya?", &id);
    pl_free(cclasses);

    *pcc = id;

    return selected;
}// get_cclass


// Let the user select a gender.
static bool
get_gender(enum GENDER *pgender, bool spouse) {
    struct PickList *genders = pl_malloc();

    pl_add(genders, FEMALE,     'f', spouse ? "woman" : "female");
    pl_add(genders, MALE,       'm', spouse ? "man" : "male");
    pl_add(genders, NONBINARY,  'n', spouse ? "non-binary person":"non-binary");

    int id = 0;
    const char *heading = spouse     ?
        "And you're married to a..." :
        "So, what are ya?";
    bool selected = pick_item(genders, heading, &id);
    pl_free(genders);

    *pgender = id;

    return selected;
}// get_gender


// Let the user select a character class and gender.
bool
get_player_type(char *playername, size_t playername_len,
                enum CHAR_CLASS *pcc, enum GENDER *pgender,
                enum GENDER *sgender) {
    const char *prompt = "Who are ya?";

    if (!fullscreen_prompt(prompt, playername, playername_len)) {
        return false;
    }

    if (!get_cclass(pcc)) { return false; }
    if (!get_gender(pgender, false)) { return false; }
    if (!get_gender(sgender, true)) { return false; }

    return true;
}/* get_player_type*/


// Fetch a keystroke (via ConsoleWin) and return it, displaying the
// cursor while waiting.
static int
cursor_getch() {
    int key;
    curs_set(2);
    key = wgetch(ConsoleWin);
    curs_set(0);

    return key;
}


/* Given a string with various characters surrounded by parens, prompt
 * the user for one of those characters and return it.  ESC means quit
 * and returns 0. */
char
prompt(const char *question) {
    bool seeking[255];
    int n;

    memset(seeking, 0, sizeof(seeking));

    /* Look for requested keystrokes. */
    char lastkey = '\0';
    for (n = 0; question[n]; n++) {
        if (question[n] == '(' && question[n+1] && question[n+2] == ')') {
            lastkey = question[n+1];
            seeking[(int)lastkey] = true;
        }/* if */
    }/* for */

    /* Wait for the user to select an item. */
    int key;
    say("%s ", question);
    for (;;) {
        key = cursor_getch();

        // Ignore invalid keycodes
        if (key < 0 || key > sizeof(seeking)/sizeof(bool)) continue;

        // We're done if this is one of the keys being sought
        if (seeking[key]) { break; }

        // And treat ESC as equivalent to the last key
        if (key == ESC) {
            key = lastkey;
            break;
        }
    }/* for */

    say("%c\n", isprint(key) ? key : ' ');
    return key;
}/* prompt*/

/* Prompt for confirmation and return true for yes and false for
 * no/Escape. */
bool
confirm(const char *question) {
    char *allowed = "yn";
    int i;

    say("%s [yn] ", question);

    do {
        curs_set(2);

        i = cursor_getch();
        i = isalpha(i) ? tolower(i) : i;
        if (i == ESC) { i = 'n'; }

        curs_set(0);

        say("%c", i);
    } while (strchr(allowed, i) == NULL);

    say("\n", i);
    return i == 'y';
}/* confirm*/


/* Display a one-line prompt asking the user to select an item from
 * the inventory.  'action' is what the item is to be used for
 * (e.g. "wield"), 'candidates' is the set of allowed lowercase
 * letters that may be returned, 'dashForNone' and 'allowGold' add '-'
 * and '.' to the allowed item list.  Returns the item selected or 0
 * if the user cancelled. */
char
quickinv(const char *action, const char *candidates, bool dashForNone,
         bool allowGold) {
    for (;;) {
        int n, ch;

        say("What do you want to %s? [%s/?%s%s] ",
            action, candidates,
            dashForNone ? "/-" : "",
            allowGold ? "/." : "");

        ch = cursor_getch();

        say("%c\n", isprint(ch) ? ch : '?');

        if (isspace (ch) || ch == ESC) return 0;
        if ((allowGold && ch == '.') || (dashForNone && ch == '-') || ch == '?')
            return ch;

        for (n = 0; candidates[n]; n++) {
            if (ch == candidates[n]) return ch;
        }

        say("Invalid! ");
    }/* for */
}/* quickinv*/


/* Prompt for text input and write the result to 'result', limited by
 * 'maxSize' which should be the size of 'result' and must be greater
 * than zero.  'result' is guaranteed to be null-terminated.  If the
 * user hits ESC, empties the string and returns false.  Otherwise,
 * returns true. */
bool
stringPrompt(const char *question, char *result, size_t maxSize) {
    char c;
    size_t index;

    ASSERT(maxSize > 0);

    say("%s", question);

    for (index = 0; index < maxSize - 1;) {
        c = norm_return( cursor_getch() );

       if (is_backspace(c)) {
           if (index > 0) {
               backspace();
               --index;
           }
           continue;
       }// if

        if (c == ESC) {
            result[0] = 0;
            return false;
        }/* if */

        if (c == '\n') {
            say("\n");
            break;
        }/* if */

        result[index++] = c;
        say("%c", c);
    }/* for */

    result[index++] = 0;
    return true;
}/* stringPrompt*/



void
promptToContinue() {
    say("Press ENTER, ESCAPE or SPACE to continue:");

    while(true) {
        char c = norm_return( wgetch(ConsoleWin) );

        if (c == ESC || c == '\n' || c == ' ') {
            return;
        }/* if */
    }/* for */
}// promptToContinue



/* Ask the user to enter an integer between 0 and max and return it.  Return
 * -1 for cancel (via ESC).  If the user just hits enter, use
 * defaultValue.  If 'max' is negative, there is no upper limit. */
long
numPrompt(const char *question, long defaultValue, long max) {
    long result;
    bool success;

    result = numPromptAll(question, defaultValue, 0, max, &success);
    return success ? result : -1;
}/* numPrompt*/



/* Ask the user for an integer between 'min' and 'max'.  If they are
 * equal, they are ignored and any integer is allowed.  'question' is
 * the text with which to prompt the user.  If the user cancels the
 * request, '*success' is set to false and -1 is returned.  Otherwise,
 * it is set to true and the actual value is returned.  (This lets the
 * user enter -1.) */
long
numPromptAll(const char *question, long defaultValue, long min, long max,
             bool *success) {
    char buffer[80];

    *success = true;
    while (1) {
        long result;
        char *endptr;

        say("%s [%d] ", question, defaultValue);
        if (!stringPrompt("", buffer, sizeof(buffer))) {
            *success = false;
            return -1;
        }/* if */

        if (*buffer == 0) return defaultValue;

        result = strtol(buffer, &endptr, 10);
        if (*endptr == 0 && (min == max || (result >= min && result <= max))) {
            return result;
        }/* if */
        say("\n");
    }/* while */
}/* numPrompt*/

/* Prompt the user for a direction and return it.  DIR_CANCEL means
 * cancel.  This is only an option if allowCancel is true. */
DIRECTION
promptdir(bool allowCancel) {
    bool try = true;
    DIRECTION dir = DIR_CANCEL;

    while (try) {
        try = false;

        say("In what direction? ");
        switch (cursor_getch()) {
        case 'h': dir = DIR_WEST; break;
        case 'l': dir = DIR_EAST; break;
        case 'j': dir = DIR_SOUTH; break;
        case 'k': dir = DIR_NORTH; break;
        case 'u': dir = DIR_NORTHEAST; break;
        case 'y': dir = DIR_NORTHWEST; break;
        case 'n': dir = DIR_SOUTHEAST; break;
        case 'b': dir = DIR_SOUTHWEST; break;
        case ESC:
            if (allowCancel) {
                say("cancelled.\n");
                dir = DIR_CANCEL;
                break;
            }

        default:
            try = true;
        }/* switch */
    }/* while */

    say("\n");
    return dir;
}


/* Display multiple lines of text on the screen until the user
 * dismisses them.  Splits on newlines.  You must provide at least one
 * string because of ISO C vararg rules. */
void
billboard(bool center, const char *heading, ...) {
    va_list ap; /* pointer for variable argument list */
    struct TextBuffer *tb;

    tb = tb_malloc(INF_BUFFER, 80);

    // The first line.  (This is because ISO C forbids using types
    // that can be implicitly promoted in a vararg argument list as
    // the start of a vararg list.  bool gets promoted to int, which
    // issues a warning, which I make into an error via -Werror, so
    // here we are.)
    tb_appendline(tb, heading);

    va_start(ap, heading);

    for (;;) {
        const char *line;

        line = va_arg(ap, char *);
        if (!line) break;

        tb_appendline(tb, line);
    }/* for */

    va_end(ap);

    if (center) {
        tb_center_all(tb);
    }// if

    showpages(tb);

    tb_free(tb);
}/* billboard*/


// Redraw the console with last CONSOLE_H lines of ConsoleBuffer and
// move the cursor to the end of the last non-empty line written. (We
// do this so that the cursor is in the right place when prompting.)
static void
update_msg(bool force_redraw) {
    static int last_update_line = 0;

    debugCycleBackground(ConsoleWin);

    // If we just need to append one line to the bottom of the buffer
    // AND it's the start of a new line, we use the scroller and add
    // just the line.  This is relatively low-hanging fruit when
    // optimizing for network use.
    if (!force_redraw &&
        ScrollbackPos == 0 &&
        tb_newline(ConsoleBuffer) &&
        tb_total_lines(ConsoleBuffer) - last_update_line == 1) {
        int ypos = tb_num_lines(ConsoleBuffer) - 1;

        if (ypos >= CONSOLE_H) {
            scroll(ConsoleWin);
            ypos = CONSOLE_H - 1;
        }

        const char *line = tb_getlastn(ConsoleBuffer, 0, 0, 1);
        mvwaddnstr(ConsoleWin, ypos, 0, line, SCREEN_W);

        goto done;
    }// if

    // Otherwise, redraw the entire window
    wclear(ConsoleWin);

    int lastx = 0, lasty = 0;
    for (int n = 0; n < CONSOLE_H; n++) {
        const char *line = tb_getlastn(ConsoleBuffer,n,ScrollbackPos,CONSOLE_H);
        mvwaddnstr(ConsoleWin, n, 0, line, SCREEN_W);
        if (*line) {
            lastx = strlen(line);
            lasty = n;
        }
    }/* for */

    // Move the cursor to the end of the last non-empty line.
    wmove(ConsoleWin, lasty, lastx);

done:
    last_update_line = tb_total_lines(ConsoleBuffer);
}/* update_msg*/


void
scroll_back() {
    if (!ConsoleBuffer ||
        ScrollbackPos >= tb_num_lines(ConsoleBuffer) - CONSOLE_H
        )
    {
        return;
    }

    ++ScrollbackPos;
    update_msg(true);
}// scroll_back

void
scroll_forward() {
    if (!ConsoleBuffer || ScrollbackPos <= 0) { return; }

    --ScrollbackPos;
    update_msg(true);
}// scroll_forward

void
say(const char *fmt, ...) {
    va_list ap; /* pointer for variable argument list */
    char buf[5 * 80];

    // We initialize this lazily so that say() can be used before the
    // UI is set up.
    if (!ConsoleBuffer) {
        ConsoleBuffer = tb_malloc (
            300,
            SCREEN_W);
    }// if

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    tb_append(ConsoleBuffer, buf);

    // If say() was called before UI initialization, we skip the
    // update.
    if (ConsoleWin) {
        bool force = (ScrollbackPos != 0);
        ScrollbackPos = 0;      // Undo any scrolling.
        update_msg(force);
    }// if
}/* say*/

// Delete the last character of the last line in the display
static void
backspace() {
    ASSERT(ConsoleBuffer && ConsoleWin);

    tb_backspace_last_line(ConsoleBuffer);

    // We just redraw everything.  This could be optimized but it
    // happens rarely enough that I'm not going to worry about it
    // right now.

    ScrollbackPos = 0;      // Undo any scrolling.
    update_msg(true);
}// backspace


// Update the visible display of active effects (e.g. stealth).  If
// 'force' is false, it may skip drawing some or all of the text that
// is not present.
//
// (Currently, it skips redrawing if no effects have changed since the
// last call.)
void
show_indicators(bool force) {
    static uint32_t last_update = (uint32_t)(-1);

    struct {
        bool show;
        const char *label;
    } indicators [] = {
        {!!UU.stealth,         "Stealth"},
        {!!UU.undeadpro,       "Undead Pro"},
        {!!UU.spiritpro,       "Spirit Pro"},
        {!!UU.charmcount,      "Charm"},
        {!!UU.timestop,        "Time Stop"},
        {!!UU.holdmonst,       "Hold Monst"},
        {!!UU.giantstr,        "Giant Str"},
        {!!UU.fireresistance,  "Fire Resist"},
        {!!UU.dexCount,        "Dexterity"},
        {!!UU.strcount,        "Strength"},
        {!!UU.scaremonst,      "Scare"},
        {!!UU.hasteSelf,       "Haste Self"},
        {!!UU.cancellation,    "Cancel"},
        {!!UU.invisibility,    "Invisible"},
        {!!UU.altpro,          "Shielded"},
        {!!UU.protectionTime,  "Protected"},
        {!!UU.wtw,             "Wall-Walk"},

        // Note: must be less than 32 items so that the initial
        // last_update will never equal the current this_update.
        {!!false, NULL}
    };

    uint32_t this_update = 0;
    for (int i = 0; indicators[i].label; i++) {
        if (indicators[i].show) { this_update |= 1; }
        this_update <<= 1;
    }

    bool status_changed = this_update != last_update;
    last_update = this_update;

    // If nothing changed (and it's not required), do nothing.
    if (!force && !status_changed) { return; }

    // Update the indicator display.
    for (int i = 0; indicators[i].label; i++) {
        const char *str =
            indicators[i].show ? indicators[i].label
                               : "                      ";
        debugCycleBackground(IndWin);
        mvwaddnstr(IndWin, i, 2, str, IndWidth - 2);
    }/* for */
}/* show_indicators*/


/* routine to pause for n milliseconds.  Also forces an update. */
void
nap(int x) {
    update_display();
    if (x <= 0 || GameSettings.nonap) return;
    napms(x);
}/* nap*/

// Flash the screen to get the user's attention
void
headsup() {
    update_display();
    if (!GameSettings.nobeep) {
        flash();
    }// if
}// headsup


// Print a message (presumably an error report of some kind) without
// using the UI.  This is typically used when the UI hasn't been
// initialized via init_ui() or after it's been torn down via
// teardown_ui().  We prefer this to regular printf because some
// platforms don't provide stdout.
void
notify(const char *fmt, ...) {
    va_list ap; /* pointer for variable argument list */

    char buffer[512];

    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    show_notification_msg(buffer);
}
