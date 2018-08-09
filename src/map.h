// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module contains code and data structures to manage the
// individual maps.  Save and restore functions also live here.

#ifndef HDR_GUARD_MAP_H
#define HDR_GUARD_MAP_H

#include "constants.h"
#include "monster.h"


#define MAX_STOLEN  32

#define MAXLEVEL 16     /*  max # levels in the dungeon + 1 */
#define MAXVLEVEL 5     /*  max # of levels in the volcano */

#define NLEVELS (MAXLEVEL + MAXVLEVEL)
#define DBOTTOM (MAXLEVEL - 1)
#define VTOP    (DBOTTOM + 1)
#define VBOTTOM (MAXLEVEL + MAXVLEVEL - 1)

#define MAXX 67
#define MAXY 17

#define CAVE_EXIT_X 33
#define CAVE_EXIT_Y (MAXY - 1)

struct MapSquare {
    struct Monster mon;
    struct Object obj;
    int know        :1;
};

struct Level {
    struct MapSquare map[MAXX][MAXY];
    bool exists;
    struct Object stolen[MAX_STOLEN];
    unsigned numStolen;
};

extern struct MapSquare *Map[MAXX];
struct Level *Lev;

int getlevel(void);
const char *getlevelname(void);
void setlevel(int);
bool savegame_to_file(FILE *fh);
bool restore_from_file(FILE *fh, bool *wrongFileVersion);
void init_cells(void);
void enlighten(int xrange, int yrange);
void add_to_stolen(struct Object thing, struct Level *lev);
struct Object remove_stolen(struct Level *lev);
bool findobj(uint8_t type, int *x, int *y);
void remake_map_keeping_contents(void);
void createitem(int x, int y, int it, int arg);
void something(int x, int y, int lev);
bool cgood(int x, int y, bool itm, bool monst);



// Restrict x,y to the bounds of the map
#define VXY(x,y)                                \
    do {                                        \
        if((x) < 0) (x) = 0;                    \
        if((y) < 0) (y) = 0;                    \
        if((x) >= MAXX) (x) = MAXX-1;           \
        if((y) >= MAXY) (y) = MAXY-1;           \
    } while (0)





#endif
