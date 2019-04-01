// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.



#include "internal_assert.h"
#include "sphere.h"
#include "display.h"
#include "store.h"
#include "create.h"
#include "game.h"
#include "look.h"
#include "os.h"
#include "version_info.h"

#include "map.h"

#include <errno.h>

// PLATFORM_ID is an ID specific to the OS+CPU so we can detect
// incompatible save files.  It needs to get set on the command line.
// Currently uses the result of `uname -psr` but can be anything
// provided that it's specific to that architecture and OS.
#ifndef PLATFORM_ID
#   error "PLATFORM_ID is undefined."
#endif

#define RELARN_SAVE_ID "ReLarn " VERSION " " PLATFORM_ID "\n"

static void bwrite(FILE *fh, unsigned int *sum, char *buf, size_t bufSize,
                   bool *errorOccurred);
static void bread(FILE *fh, unsigned int *sum, char *buf, size_t bufSize,
                  bool *errorOccurred);
static void setlevptrs (void);

static void newcavelevel (void);
static void makemaze(int lev);
static bool cannedlevel(int lev);
static void treasureroom(int lv);
static void troom(int lv, int xsize, int ysize, int tx, int ty,
                  enum DOORTRAP_RISK dtr);
static void makeobject(int j);
static void fillmroom(int n, int what, int arg);
static void froom(int n, int itm, int arg);
static void fillroom(struct Object obj);
static void sethp(bool firstVisit);
static void checkgen(void);
static void eat(int xx, int yy);
static unsigned int sum(unsigned char *data, int n);

/* The current level: */
static int LevelNum = -1;

/* All maps: */
struct MapSquare *Map[MAXX];
static struct Level Levels[NLEVELS];

/* Return the current level. -1 means the map hasn't been created yet. */
int
getlevel() {
    return LevelNum;
}/* getlevel*/

const char *
getlevelname() {
    static const char *levelname[] =  {
        " H"," 1"," 2"," 3"," 4"," 5",
        " 6"," 7"," 8"," 9","10","11","12","13","14","15",
        "V1","V2","V3","V4","V5"
    };
    return levelname[LevelNum];
}// getlevelname

/* destroy object at present location */
void
udelobj() {
    Map[UU.x][UU.y].obj = NULL_OBJ;
    see_and_update_fov();
}/* udelobj*/


void
setlevel(int newlevel) {
    LevelNum = newlevel;

    /* Reset teleflag if this is level 0 since now, we know where we
     * are. */
    if (newlevel == 0) {
        UU.teleflag = false;
    }/* if */

    /* Set the global level and map pointers. */
    setlevptrs();

    /* We'll probably need to redraw the display. */
    force_full_update();

    /* restore the new level if it exists. */
    if (Lev->exists) {
        sethp(false);
        checkgen();
        return;
    }/* if */

    /* Otherwise, force the creation of the current level. */
    newcavelevel();
}/* setlevel*/


/* Set the global pointers to the current map and level structures. */
static void
setlevptrs () {
    int n;

    Lev = &Levels[LevelNum];

    /* Make map point to the map in Lev. */
    for (n = 0; n < MAXX; n++) {
        Map[n] = &(Lev->map[n][0]);
    }/* for */
}/* setlevptrs */


/* Add 'thing' to the list of stolen items in the Level referenced by
 * 'lev'.  If the stolen object list is full, pick an item at random
 * and replace it with 'thing'. */
void
add_to_stolen(struct Object thing, struct Level *lev) {
    if (lev->numStolen < MAX_STOLEN) {
        lev->stolen[lev->numStolen] = thing;
        lev->numStolen++;
        return;
    }/* if */

    lev->stolen[rund(MAX_STOLEN)] = thing;
}/* add_to_stolen*/


/* Return a random object from the stolen items list and remove it
 * from the list.  If the list is empty, return NULL_OBJ. */
struct Object
remove_stolen(struct Level *lev) {
    struct Object result = NULL_OBJ;
    unsigned index;

    if (lev->numStolen <= 0) {
        return result;
    }/* if */

    /* Pick an item at random and store it in 'result' for return.
     * Then, move the last item in the list into the slot occupied by
     * the returned item and decrement the count. */

    index = rund(lev->numStolen);
    ASSERT (index < lev->numStolen);    // sanity check against bad coding
    result = lev->stolen[index];
    lev->stolen[index] = lev->stolen[lev->numStolen - 1];

    lev->numStolen--;

    return result;
}/* remove_stolen*/






// Save the game to 'fh'.  Return true on success, false if an error
// occurred.
bool
savegame_to_file(FILE *fh) {
    int i;
    char genocided; /* To keep save files compatible with old struct MonstTypeData */
    struct sphere *sp;
    unsigned int filesum = 0;
    char *save_id = RELARN_SAVE_ID;
    bool known[OBJ_COUNT];

    bool errorOccurred = false;

    bwrite(fh, &filesum, save_id, strlen(save_id) + 1, &errorOccurred);

    // To do: only write out levels which exist.  (Then again, the
    // empty levels compress to almost nothing.)
    bwrite(fh, &filesum, (char *)Levels, sizeof(Levels), &errorOccurred);

    bwrite(fh, &filesum, (char *)&UU, sizeof(UU), &errorOccurred);
    bwrite(fh, &filesum, (char *)&GS, sizeof(GS), &errorOccurred);
    bwrite(fh, &filesum, (char *)&LevelNum, sizeof(LevelNum), &errorOccurred);
    bwrite(fh, &filesum, (char *)Invent, sizeof(struct Object) * IVENSIZE,
           &errorOccurred);

    for (i = 0; i < OBJ_COUNT; i++) {
        known[i] = Types[i].isKnown;
    }
    bwrite(fh, &filesum, (char *)known, sizeof(known), &errorOccurred);
    bwrite(fh, &filesum, (char *)&ShopInventSz, sizeof(ShopInventSz), &errorOccurred);
    bwrite(fh, &filesum, (char *)ShopInvent, sizeof(struct StoreItem) * ShopInventSz,
           &errorOccurred);

    /* Write genocide status */
    for (i = 0; i < MAXCREATURE; i++)  {
        genocided = ((MonType[i].flags & FL_GENOCIDED) != 0);
        bwrite(fh, &filesum, &genocided, sizeof(genocided), &errorOccurred);
    }

    /* save spheres of annihilation */
    sp=spheres;
    for (i = 0; i < UU.sphcast; i++) {
        bwrite(fh, &filesum, (char * )sp, sizeof(struct sphere), &errorOccurred);
        sp = sp->p;
    }

    /* file sum */
    bwrite(fh, &filesum, (char *)&filesum, sizeof(filesum), &errorOccurred);

    return !errorOccurred;
}/* savegame_to_file*/


// Load saved game from 'fh'.  Returns true on success, false on error.
//
// If the input file was from an incompatible version of relarn and
// wrongFileVersion is not NULL, it will also set *wrongFileVersion to
// true (and to false otherwise).
bool
restore_from_file(FILE *fh, bool *wrongFileVersion) {
    int i;
    char genocided; /* To keep save files compatible with old struct MonstTypeData */
    unsigned int thesum, asum;
    struct sphere *sp,*splast;
    unsigned int filesum = 0;
    char save_id_maybe[80];
    size_t sid_len;
    bool known[OBJ_COUNT];
    bool fileErr = false;

    if (wrongFileVersion) { *wrongFileVersion = false; }

    sid_len = strlen(RELARN_SAVE_ID);
    ASSERT (sid_len < sizeof(save_id_maybe));

    bread (fh, &filesum, save_id_maybe, sid_len + 1, &fileErr);
    save_id_maybe[sid_len] = 0;  /* Ensure null-termination. */
    if (strcmp(save_id_maybe, RELARN_SAVE_ID) != 0) {
        if (wrongFileVersion) { *wrongFileVersion = true; }
        return false;
    }/* if */

    bread(fh, &filesum, (char *)Levels, sizeof(Levels), &fileErr);
    bread(fh, &filesum, (char *)&UU, sizeof(UU), &fileErr);
    bread(fh, &filesum, (char *)&GS, sizeof(GS), &fileErr);
    bread(fh, &filesum, (char *)&LevelNum, sizeof(LevelNum), &fileErr);
    bread(fh, &filesum, (char * )Invent, sizeof(struct Object) * IVENSIZE,
          &fileErr);
    bread(fh, &filesum, (char *)known, sizeof(known), &fileErr);

    bread(fh, &filesum, (char *)&ShopInventSz, sizeof(ShopInventSz), &fileErr);
    if(ShopInventSz >= OBJ_COUNT) { return false; }

    bread(fh, &filesum, (char*)ShopInvent, sizeof(struct StoreItem)*ShopInventSz,
          &fileErr);

    /* Read genocide info into monster flags */
    for (i = 0; i < MAXCREATURE; i++)  {
        bread(fh, &filesum,  &genocided, sizeof(genocided), &fileErr);
        if (genocided) MonType[i].flags |= FL_GENOCIDED;
    }

    /* get spheres of annihilation */
    for (i = 0; i < UU.sphcast; i++) {
        sp = xmalloc(sizeof(struct sphere));
        bread(fh, &filesum,  (char *)sp, sizeof(struct sphere), &fileErr);
        if (i==0) {
            spheres = sp;
            splast = sp;
            sp = sp->p;
        } else {
            splast->p = sp;
            splast = sp;
            sp = sp->p;
        }
    }

    // Compute the checksum and return if they don't match
    thesum = filesum;   /* sum of everything so far */
    bread(fh, &filesum, (char *)&asum, sizeof(asum), &fileErr);

    if (asum != thesum) {
        return false;
    }/* if */

    //
    // Now, do post-load processing
    //

    if (UU.hp <= 0) {
        return false;
    }

    for (i = 0; i < OBJ_COUNT; i++) {
        Types[i].isKnown = known[i];
    }/* for */

    /* Set the global level and map pointers. */
    setlevptrs();

    /*
     *  closedoor() in action.c sets dropflag to stop the player being
     *  asked to re-open a door they just closed.  However, if they
     *  save the game before moving off that square, dropflag is lost.
     *  We restore it here.
     */
    if (Map[UU.x][UU.y].obj.type == OCLOSEDDOOR) {
        cancel_look();
    }/* if */

    return true;
}/* restore_from_file*/

// Write 'num' bytes of 'buf' to 'fh' unless *errorOccurred is true in
// which case do nothing.  If an I/O error occurrs, set *errorOccurred
// to true.  Also update filesum with the file checksum.
static void
bwrite(FILE *fh, unsigned int *filesum, char *buf, size_t num,
       bool *errorOccurred)
{
    if (*errorOccurred) { return; }

    int nwrote = fwrite(buf, 1, num, fh);
    if (nwrote != num) { *errorOccurred = true; }

    *filesum += sum((unsigned char *)buf, num);
}/* bwrite*/


// Like bwrite, but reads.
//
// Also: % Baby I'm-a want your savefile. %
static void
bread(FILE *fh, unsigned int *filesum, char *buf, size_t bufSize,
      bool *errorOccurred)
{
    if (*errorOccurred) { return; }

    int nread = fread(buf, 1, bufSize, fh);
    if (nread != bufSize) { *errorOccurred = true; }

    *filesum += sum((unsigned char *)buf, bufSize);
}/* bread*/


/* Compute a checksum for 'data'. */
static unsigned int
sum(unsigned char *data, int n) {
    unsigned int sum;
    int c, nb;

    sum = nb = 0;
    while (nb++ < n) {
        c = *data++;
        if (sum & 01)
            sum = (sum >> 1) + 0x8000;
        else
            sum >>= 1;
        sum += c;
        sum &= 0xFFFF;
    }
    return sum;
}/* sum*/


/*                     Map creation                       */

/* Create the current cave level.  Must not already exist. */
static void
newcavelevel () {
    ASSERT(!Lev->exists);

    /* Create the maze; either fetch it from a data file or generate
     * it. */
    int lvl = getlevel();
    if (lvl == 0 || !cannedlevel(lvl)) {
        makemaze(lvl);
    }// if

    // Forget everything
    set_reveal(false);

    makeobject(getlevel());
    Lev->exists = true;   /* first time here */
    sethp(true);

    if (getlevel() == 0) {
        set_reveal(true);
        force_full_update();
    }/* if */

    checkgen(); /* wipe out any genocided monsters */
}/* newcavelevel */


/*
 *  makemaze(level)
 *  int level;
 *
 *  subroutine to make the caverns for a given level.  only walls are made.
 */

void
makemaze (int lev) {
    int mx,mxl,mxh,my,myl,myh,tmp2;
    int z;

    /* fill up maze */
    {
        struct Object wallish = lev == 0 ? NULL_OBJ : obj(OWALL, 0);
        for (int i=0; i<MAXY; i++) {
            for (int j=0; j<MAXX; j++) {
                Map[j][i].obj = wallish;
            }/* for */
        }/* for */
    }

    /* don't need to do anymore for level 0 */
    if (lev==0) { return; }

    eat(1,1);

    /*  now for open spaces -- not on level 15 or V5 */
    if (lev != DBOTTOM && lev != VBOTTOM) {
        tmp2 = rnd(3)+3;
        for (int tmp=0; tmp<tmp2; tmp++) {
            my = rnd(11)+2;
            myl = my - rnd(2);
            myh = my + rnd(2);
            if (lev <= DBOTTOM) {   /* in dungeon */
                mx = rnd(44)+5;
                mxl = mx - rnd(4);
                mxh = mx + rnd(12)+3;
                z=0;
            }
            else {  /* in volcano */
                mx = rnd(60)+3;
                mxl = mx - rnd(2);
                mxh = mx + rnd(2);
                z = makemonst(lev);
            }
            for (int i = mxl; i < mxh; i++) {
                for (int j = myl; j < myh; j++) {
                    Map[i][j].obj = NULL_OBJ;
                    if (z) { Map[i][j].mon = mk_mon(z); }
                }/* for */
            }/* for */
        }/* for */
    }/* if */

    if (lev!=DBOTTOM && lev!=VBOTTOM) {
        my = rnd(MAXY-2);
        for (int i = 1; i < MAXX-1; i++) {
            Map[i][my].obj = NULL_OBJ;
        }
    }

    /* no treasure rooms above level 5 */
    if (lev>4) {
        treasureroom(lev);
    }
}/* makemaze */


// Create a new maze; used for Alter Reality spell
void
remake_map_keeping_contents() {
    struct isave {
        enum {ITEM, MONSTER} type;
        union {
            struct Object o;
            struct Monster m;
        } i;
    } *save;
    int sc = 0;         /* # items saved */

    save = xmalloc(sizeof(struct isave) * MAXX * MAXY * 2);

    /* save all items and monsters and fill the level with walls */
    for (int y = 0; y < MAXY; y++) {
        for (int x = 0; x < MAXX; x++) {
            struct MapSquare *pt = &Map[x][y];
            int item = pt->obj.type;

            if (item && item != OWALL && item != OANNIHILATION && item!=OEXIT){
                save[sc].type = ITEM;
                save[sc].i.o = Map[x][y].obj;
                ++sc;
            }/* if */

            if (pt->mon.id) {
                save[sc].type = MONSTER;
                save[sc].i.m = Map[x][y].mon;
                ++sc;
            }/* if */

            pt->obj = obj(OWALL, 0);
            pt->mon = NULL_MON;
            forget_at(x, y);
        }/* for */
    }/* for */

    /* Create a new level. */
    eat(1, 1);

    /* Create the exit if this is level 1. */
    if (getlevel() == 1) {
        Map[CAVE_EXIT_X][CAVE_EXIT_Y].obj = obj(OEXIT, 0);
    }

    for (int j = rnd(MAXY - 2), i = 1; i < MAXX - 1; i++) {
        Map[i][j].obj = obj(ONONE, 0);
    }

    /* put objects back in level */
    for (; sc >= 0;  --sc) {
        int tries = 100;
        int x = 1, y = 1;

        if (save[sc].type == ITEM) {
            while (--tries > 0 && Map[x][y].obj.type) {
                x = rnd(MAXX - 1);
                y = rnd(MAXY - 1);
            }/* while */

            if (tries) {
                Map[x][y].obj = save[sc].i.o;
            }/* if */
        } else {    /* put monsters back in */
            while (--tries > 0 && (Map[x][y].obj.type == OWALL
                                      || Map[x][y].mon.id)) {
                x = rnd(MAXX - 1);
                y = rnd(MAXY - 1);
            }/* while */

            if (tries) {
                Map[x][y].mon = save[sc].i.m;
            }/* if */
        }/* if .. else*/
    }/* for */

    free((char *) save);
}// remake_map_keeping_contents


// Maze generation function.  Given a filled-in level, removes
// material in a random direction before recursing.
static void
eat (int xx, int yy) {
    int dir,try;

    dir = rnd(4);
    try=2;
    while (try) {
        switch(dir) {
        case 1:
            if (xx <= 2) break; /*  west    */
            if ((Map[xx-1][yy].obj.type!=OWALL) || (Map[xx-2][yy].obj.type!=OWALL))
                break;
            Map[xx-1][yy].obj = NULL_OBJ;
            Map[xx-2][yy].obj = NULL_OBJ;
            eat(xx-2,yy);
            break;
        case 2:
            if (xx >= MAXX-3) break;  /*    east    */
            if ((Map[xx+1][yy].obj.type!=OWALL) || (Map[xx+2][yy].obj.type!=OWALL))
                break;
            Map[xx+1][yy].obj = NULL_OBJ;
            Map[xx+2][yy].obj = NULL_OBJ;
            eat(xx+2,yy);
            break;
        case 3:
            if (yy <= 2) break; /*  south   */
            if ((Map[xx][yy-1].obj.type!=OWALL) || (Map[xx][yy-2].obj.type!=OWALL))
                break;
            Map[xx][yy-1].obj = NULL_OBJ;
            Map[xx][yy-2].obj = NULL_OBJ;
            eat(xx,yy-2);
            break;
        case 4:
            if (yy >= MAXY-3 ) break;   /*north */
            if ((Map[xx][yy+1].obj.type!=OWALL) || (Map[xx][yy+2].obj.type!=OWALL))
                break;
            Map[xx][yy+1].obj = NULL_OBJ;
            Map[xx][yy+2].obj = NULL_OBJ;
            eat(xx,yy+2);
            break;
        };
        if (++dir > 4)  {
            dir=1;
            --try;
        }
    }/* while */
}/* eat */


/*
 *  function to read in a maze from a data file
 *
 *  Only read in a maze 50% of time.
 *
 *  Format of maze data file:
 *              1st character = # of mazes in file (ascii digit)
 *              For each maze:
 *                  18 lines (1st 17 used)
 *                  67 characters per line
 *
 *  line seperating maps must be single newline character
 *
 *  Special characters in maze data file:
 *
 *      #   wall            D   door
 *      .   random monster      ~   eye of larn
 *      !   cure dianthroritis  -   random object
 *
 *
 *  Returns true if maze was created, false if not (for any rason)
 */

static bool
cannedlevel(int lev) {
    FILE *fp;
    char *row, buf[128];

    /* only read a maze from file around half the time */
    if (lev != DBOTTOM && lev != VBOTTOM && rnd(100) < 50) {
        return false;
    }// if

    fp = fopen(levels_path(), "r");
    if (!fp) {
        // We should really be checking for this at startup and
        // refusing to start if the file is missing; then make this a
        // fatal error.  But for now, a subtle error message is
        // easiest.
        say("%s",
            GS.wizardMode                   ?
            "Error opening levels file!\n"  :
            "You feel vague existential unease.\n");
        return false;
    }/* if */

    /*
    **      Umap format
    **  - lines must be MAXX characters long
    **  - must be MAXY characters per map
    **  - each map must be seperated by 1 blank line
    **    (a single newline character)
    */
    {
        int idx = rund(20);
        fseek(fp, (long)(idx * ((MAXX * MAXY)+MAXY+1)), 0);
        if (GS.wizardMode) {
            say("Loading canned level %d.\n", idx);
        }
    }

    for (int y = 0; y < MAXY; y++) {
        if ((row = fgets(buf, 128, fp)) == (char *)NULL) {
            if (GS.wizardMode) {
                say("IO error when reading map: %s\n", strerror(errno));
            }
            fclose(fp);
            return false;
        }

        for (int x = 0; x < MAXX; x++) {
            struct Object nob = NULL_OBJ;
            int mit = 0;

            switch(*row++) {
            case '#':
                nob = obj(OWALL, 0);
                break;
            case 'D':
                nob = door(DTO_LOW);
                break;
            case '~':
                if (lev!=DBOTTOM)
                    break;
                nob = obj(OLARNEYE, 0);
                mit = DEMONPRINCE;
                break;
            case '!':
                if (lev!=VBOTTOM)
                    break;
                nob = obj(OPCUREDIANTH, 0);
                mit = DEMONKING;
                break;
            case '.':
                if (lev<=DBOTTOM-5)  break;
                mit = makemonst(lev+1);
                break;
            case '-':
                nob = newobject(lev+1);
                break;
            };
            Map[x][y].obj = nob;
            Map[x][y].mon = mk_mon(mit);
        }// for
    }// for

    fclose(fp);
    return true;
}/* cannedlevel */

/*
 *  make a treasure room on a level
 *  - level 10's treasure room has the eye in it and demon lords
 *  - level V5 has potion of cure dianthroritis and demon prince
 */
static void
treasureroom(int lv) {
    int tx,ty,xsize,ysize;

    for (tx=1+rnd(10);  tx<MAXX-10;  tx+=10)
        if ( (lv==DBOTTOM) || (lv==VBOTTOM) || rnd(10)<=2) { /* 20% chance */
            xsize = rnd(6)+3;
            ysize = rnd(3)+3;
            ty = rnd(MAXY-9)+1;  /* upper left corner of room */
            if (lv==DBOTTOM || lv==VBOTTOM)
                troom(lv, xsize, ysize, (tx=tx+rnd(MAXX-24)), ty, DTO_MEDIUM);
            else
                troom(lv, xsize, ysize, tx, ty, DTO_HIGH);
        }
}/* treasureroom*/



/*
 *  subroutine to create a treasure room of any size at a given location
 *  room is filled with objects and monsters
 *  the coordinate given is that of the upper left corner of the room
 */
static void
troom(int lv, int xsize, int ysize, int tx, int ty, enum DOORTRAP_RISK dtr) {
    int i,j;

    for (j=ty-1; j<=ty+ysize; j++)
        for (i=tx-1; i<=tx+xsize; i++)  /* clear out space for room */
            Map[i][j].obj = NULL_OBJ;
    for (j=ty; j<ty+ysize; j++)
        /* now put in the walls */
        for (i=tx; i<tx+xsize; i++) {
            Map[i][j].obj = obj(OWALL, 0);
            Map[i][j].mon = NULL_MON;
        }
    for (j=ty+1; j<ty+ysize-1; j++)
        for (i=tx+1; i<tx+xsize-1; i++) /* now clear out interior */
            Map[i][j].obj = NULL_OBJ;

    /* locate the door on the treasure room */
    switch(rnd(2))  {
    case 1:
        i = tx + rund (xsize);
        j = ty + (ysize-1) * rund(2);

        Map[i][j].obj = door(dtr);  /* on horizontal walls */
        break;
    case 2:
        i = tx + (xsize-1)*rund(2);
        j = ty + rund (ysize);

        Map[i][j].obj = door(dtr); /* on vertical walls */
        break;
    }

    int py = ty + (ysize>>1);
    for (int px = tx + 1; px <= tx + xsize - 2; px += 2) {
        for (i=0, j = rnd(6); i<=j; i++) {
            create_rnd_item(px, py, lv+2);
            createmonster_near(px, py, makemonst(lv + UU.challenge < 3 ? 2 : 4));
        }// for
    }// for
}/* troom*/


// Initialize the current level as the town level.
static void
mktown() {
    fillroom(obj(OENTRANCE,0)); /*  entrance to dungeon*/
    fillroom(obj(ODNDSTORE,0)); /*  the DND STORE   */
    fillroom(obj(OSCHOOL,0));   /*  college of Larn */
    fillroom(obj(OBANK,0));     /*  1st national bank of larn*/
    fillroom(obj(OVOLDOWN,0));  /*  volcano shaft to temple*/
    fillroom(obj(OHOME,0));     /*  the players home & family*/
    fillroom(obj(OTRADEPOST,0));/*  the trading post    */
    fillroom(obj(OLRS,0));      /*  the larn revenue service */
}/* mktown*/


/* Make the stairs and stair-like things for this level. */
static void
makestairs(int lev) {

    /* Make the volcano shaft up if this is V1 */
    if (lev==DBOTTOM+1) {   /* V1 */
        fillroom(obj(OVOLUP,0)); /* volcano shaft up from the temple */
    }/* if */

    /* Make the cave exit if this is level 1 */
    if (lev == 1) {
        Map[CAVE_EXIT_X][CAVE_EXIT_Y].obj = obj(OEXIT, 0);
    }/* if */

    /* stairs down everywhere except V1 and V2 */
    if (lev > 0 && lev != DBOTTOM && lev < VBOTTOM-2) {
        fillroom(obj(OSTAIRSDOWN,0));
    }/* if */

    /* Stairs up for all levels but the top two and V1 (which has the
     * volcano shaft). */
    if (lev > 1 && lev != VTOP) {
        fillroom(obj(OSTAIRSUP,0));
    }/* if */

    /* Maybe put the elevator up here. */
    if (! UU.has_up_elevator &&
        lev > 3 && lev != VBOTTOM && lev < DBOTTOM && rnd(100) > 85
        ) {
        fillroom(obj(OELEVATORUP,0));
        UU.has_up_elevator = true;
    }/* if */

    /* Elevator down: < lev 10, or 15 or V5 */
    if ((lev > 0 && lev <= DBOTTOM-5) || lev == DBOTTOM || lev == VBOTTOM) {
        if (! UU.has_down_elevator && rnd(100) > 85) {
            fillroom(obj(OELEVATORDOWN,0));
            UU.has_down_elevator = true;
        }/* if */
    }/* if */

    /* be sure to have pits on V3, V4, and V5 because there are no
     * stairs on those levels */
    if (lev >= VBOTTOM-2) {
        fillroom(obj(OPIT,0));
    }/* if */

}/* makestairs*/


/* Place an object of type 'id' in the current level no such item is
 * marked as existing in UU.created[], if 'cond' is true and 'inGroup'
 * is either NULL or points to a variable containing false.  If the
 * object is created, it will be marked as such in UU.created[] and
 * *inGroup will be set to true if a non-NULL pointer was given. */
static void
makeif(enum OBJECT_ID id, bool cond, bool *inGroup) {
    /* Do nothing if a previous call already set *inGroup to true. */
    if (inGroup && *inGroup) return;

    /* Skip if the condition is false. */
    if (!cond) return;

    /* Skip if the object already exists. */
    if (UU.created[id]) return;

    /* Create the object. */
    fillroom(obj(id, 0));
    if (inGroup) *inGroup = true;
    UU.created[id] = true;
}/* makeif*/


/* Place various rare artifacts on the level, if the rund() wills
 * it. */
static void
makerare(int lev) {
    bool alreadyDone = false;

    /* Dealer McDope's Pad: */
    makeif(OPAD, rnd(100) > 75, NULL);

    /* Various rare artifacts; at most one per level. */
    makeif(OBRASSLAMP,          rnd(120) < 8, &alreadyDone);
    makeif(OWWAND,              rnd(120) < 8, &alreadyDone);
    makeif(OORBOFDRAGON,        rnd(120) < 8, &alreadyDone);
    makeif(OSPIRITSCARAB,       rnd(120) < 8, &alreadyDone);
    makeif(OCUBE_of_UNDEAD,     rnd(120) < 8, &alreadyDone);
    makeif(ONOTHEFT,            rnd(120) < 8, &alreadyDone);
    makeif(OSPHTALISMAN,        rnd(120) < 8, &alreadyDone);
    makeif(OHANDofFEAR,         rnd(120) < 8, &alreadyDone);
    makeif(OORB,                rnd(120) < 8, &alreadyDone);
    makeif(OELVENCHAIN,         rnd(120) < 8, &alreadyDone);

    /* Other rare artifacts: */
    makeif(OSWORDofSLASHING,    rnd(120) < 8,               NULL);
    makeif(OHAMMER,             rnd(120) < 8,               NULL);
    makeif(OSLAYER,             lev>=10 && lev <= VBOTTOM &&
                                rnd(100) > 85-(lev-10),
                                                            NULL);
    makeif(OVORPAL,             rnd(120) < 8,               NULL);
    makeif(OPSTAFF,             lev >= 8 && lev <= 20,      NULL);

}/* makerare*/


/*
 *  subroutine to create the objects in the maze for the given level
 */
static void
makeobject (int lev) {
    int i;

    if (lev==0) {
        mktown();
        return;
    }/* if */

    makestairs(lev);

    /*  make the random objects in the maze */
    fillmroom(rund(3),OBOOK,lev);
    fillmroom(rund(3),OCOOKIE,0);
    fillmroom(rund(3),OALTAR,0);
    fillmroom(rund(3),OSTATUE,0);
    fillmroom(rund(3),OFOUNTAIN,0);
    fillmroom(rund(2),OTHRONE,0);
    fillmroom(rund(2),OMIRROR,0);
    fillmroom(rund(3),OPIT,0);

    /* be sure to have trapdoors on V3, V4, and V5 */
    if (lev >= VBOTTOM-2) fillroom(obj(OIVTRAPDOOR,0));
    fillmroom(rund(2),OIVTRAPDOOR,0);
    fillmroom(rund(2),OTRAPARROWIV,0);
    fillmroom(rnd(3)-2,OIVTELETRAP,0);
    fillmroom(rnd(3)-2,OIVDARTRAP,0);

    fillmroom(lev == 1 ? 1 : rund(2), OCHEST, lev); /* 1 chest on lev. 1 */

    if (lev<=DBOTTOM) {
        fillmroom(rund(2),ODIAMOND,rnd(10*lev+1)+10);
        fillmroom(rund(2),ORUBY,rnd(6*lev+1)+6);
        fillmroom(rund(2),OEMERALD,rnd(4*lev+1)+4);
        fillmroom(rund(2),OSAPPHIRE,rnd(3*lev+1)+2);
    }

    for (i=0; i<rnd(4)+3; i++)
        fillroom(newpotion());  /*  make a POTION   */

    for (i=0; i<rnd(5)+3; i++)
        fillroom(newscroll());  /*  make a SCROLL   */

    for (i=0; i<rnd(12)+11; i++)
        fillroom(obj(OGOLDPILE,12*rnd(lev+1)+(lev<<3)+10)); /* make GOLD */

    if (lev==8)
        fillroom(obj(OBANK2,0)); /*  branch office of the bank */

    froom(2,ORING,0);       /* a ring mail  */
    froom(1,OSTUDLEATHER,0);    /* a studded leather    */
    froom(3,OSPLINT,0);     /* a splint mail*/
    froom(5,OSHIELD,rund(3));   /* a shield */
    froom(2,OBATTLEAXE,rund(3));    /* a battle axe */
    froom(5,OLONGSWORD,rund(3));    /* a long sword */
    froom(5,OFLAIL,rund(3));    /* a flail  */
    froom(7,OSPEAR,rnd(5));     /* a spear  */
    froom(4,OREGENRING,rund(3));    /* ring of regeneration */
    froom(1,OPROTRING,rund(3)); /* ring of protection   */
    froom(2,OSTRRING,rund(5));  /* ring of strength  */
    froom(2,ORINGOFEXTRA,0);    /* ring of extra regen  */

    makerare(lev);

    /* we don't get these if the difficulty level
    ** is >= 3
    */
    if (UU.challenge<3 || (rnd(4)==3)) {
        if (lev>3) {   /* only on levels 3 or below */
            froom(3,OSWORD,rund(6));  /* sunsword */
            froom(5,O2SWORD,rnd(6));  /* a two handed sword */
            froom(3,OBELT,rund(7));   /* belt of striking   */
            froom(3,OENERGYRING,rund(6));   /* energy ring  */
            froom(4,OPLATE,rund(8));  /* platemail */
        }
    }
}/* makeobject */

/*
 *  subroutine to fill in a number of objects of the same kind
 */
static void
fillmroom (int count, int what, int arg) {
    int i;

    for (i=0; i<count; i++) {
        fillroom(obj(what,arg));
    }/* for */
}/* fillmroom */


// Place an item of type itm/arg in the level at a random location
// *IF* a random condition is met.
static void
froom(int n, int itm, int arg) {
    if (rnd(151) < n) {
        fillroom(obj(itm,arg));
    }
}/* froom*/

/*
 *  subroutine to put an object into an empty room
 *  uses a random walk
 */
static void
fillroom (struct Object obj) {
    int x,y;

    x=rnd(MAXX-2);
    y=rnd(MAXY-2);
    while (Map[x][y].obj.type) {
        x += rnd(3)-2;
        y += rnd(3)-2;
        if (x > MAXX-2)
            x=1;
        if (x < 1)
            x=MAXX-2;
        if (y > MAXY-2)
            y=1;
        if (y < 1)
            y=MAXY-2;
    }
    Map[x][y].obj = obj;
}/* fillroom */


/*
 *  creates an entire set of monsters for a level
 *  must be done when entering a new level
 *  if sethp(1) then wipe out old monsters else leave them there
 */
static void
sethp (bool firstVisit) {
    int i,j;

    // Level 0 has no monsters.
    if (getlevel() == 0) { return; }

    // Add a bunch of new monsters; more if firstVisit is true
    {
        int numMon = (getlevel() >> 1) + 1;

        if (firstVisit) { numMon += rnd(12) + 2; }

        for (int i = 0; i < numMon; i++) {
            fillmonst(makemonst(getlevel()));
        }
    }


    /*
    ** level 11 gets 1 demon lord
    ** level 12 gets 2 demon lords
    ** level 13 gets 3 demon lords
    ** level 14 gets 4 demon lords
    ** level 15 gets 5 demon lords
    */
    if ((getlevel() >= (DBOTTOM-4)) && (getlevel()<=DBOTTOM)) {
        i=getlevel()-10;
        for (j=1;j<=i;j++)
            if (fillmonst(DEMONLORD1+rund(7))==-1)
                j--;
    }
    /*
    ** level V1 gets 1 demon prince
    ** level V2 gets 2 demon princes
    ** level V3 gets 3 demon princes
    ** level V4 gets 4 demon princes
    ** level V5 gets 5 demon princes
    */
    if (getlevel() > DBOTTOM ) {
        i=getlevel()-DBOTTOM;
        for (j=1;j<=i;j++)
            if (fillmonst(DEMONPRINCE)==-1)
                j--;
    }
    positionplayer();
}/* sethp */


/*
 *  Function to destroy all genocided monsters on the present level
 */
static void
checkgen () {
    int x,y;

    for (y=0; y<MAXY; y++) {
        for (x=0; x<MAXX; x++) {
            if ((MonType[Map[x][y].mon.id].flags & FL_GENOCIDED) != 0) {
                Map[x][y].mon = NULL_MON;
            }/* if */
        }/* for */
    }/* for */
}/* checkgen */


/* Search the current map for the coordinates of an object with type
 * 'type'.  Coordinates are stored at *x, *y and true is returned on
 * success.  If nothing is found, returns false and does not modify *x
 * or *y. */
bool
findobj(uint8_t type, int *x, int *y) {
    int ix, iy;
    for (iy=0; iy < MAXY; iy++) {
        for (ix = 0; ix < MAXX; ix++) {
            if (Map[ix][iy].obj.type == type) {
                *x = ix;
                *y = iy;
                return true;
            }/* if */
        }/* for */
    }/* for */

    return false;
}/* findobj*/


/* Check x,y if it is safe to place either a new item or
 * monster. `itm` or `monst` determine which.  At least one must be
 * true. */
bool
cgood(int x, int y, bool itm, bool monst) {
    uint8_t type;

    ASSERT(itm || monst);

    if (x < 0 || x >= MAXX || y < 0 || y > MAXY)    return false;

    type = Map[x][y].obj.type;
    if (type == OWALL || type == OCLOSEDDOOR)       return false;

    if (itm     && !isnone(Map[x][y].obj))          return false;
    if (monst   && Map[x][y].mon.id != NOMONST)     return false;

    return true;
}/* cgood*/


// Choose a random spot near posX, posY that can be used to hold an
// item or monster (indicated by passing true as monst or item).  The
// location is stored in *outX,*outY.  Returns the (rectangular)
// distance from posX,posY or -1 if no location was found.
int
point_near(int posX, int posY, int *outX, int *outY, bool item, bool monst) {

    // We search for a location by searching the box surrounding
    // posX,posY.  If no location is found, we increase the box size
    // by 1 in each direction and repeat.  We keep doing this until a
    // spot is found or we exceed the size of the map.
    //
    // At each degree of distance (i.e. box size), if there are
    // multiple available spots, we pick one at random.  We do this by
    // first selecting a random cell in the search sequence and then
    // returning the first available cell there or later.  We always
    // keep the latest available cell so we can use it if there's
    // nothing valid after the random target.  (This biases the search
    // a little but that shouldn't affect the game's fairness.)

    for(int radius = 1; radius < MAXX; radius++) {
        int zonewidth = 2 * radius + 1;
        int maxSlots = 4 * zonewidth - 2;

        int rndTarget = rnd(maxSlots);  // The target index
        int lastX = -1, lastY = -1;     // The most-recent empty cell found

        // Check if we've found a valid cell at x,y; if so, exit the
        // search.  A cell is valid if it can hold the requested thing
        // and is at or past rndTarget in the search order.  Also
        // decrements rndTarget.
#define CHECK(x,y)                                  \
        if (cgood(x, y, item, monst)) {             \
            lastX = x;                              \
            lastY = y;                              \
            if (rndTarget == 0) { goto found_it; }  \
        }                                           \
        if (rndTarget > 0) { --rndTarget; }

        // Search the vertical edges
        for (int y = posY - radius; y <= posY + radius; y++) {
            CHECK(posX - radius, y);
            CHECK(posX + radius, y);
        }// for

        // Search the horizontal edges
        for (int x = posX - radius + 1; x <= posX + radius - 1; x++) {
            CHECK(x, posY - radius);
            CHECK(x, posY + radius);
        }// for

#undef CHECK

    found_it:

        if (lastX >= 0 && lastY >= 0) {
            *outX = lastX;
            *outY = lastY;
            return radius;
        }// if

    }// for

    // There are no free spots available.
    return -1;
}// point_near


// Create a new item in a free square adjacent to baseX, baseY on the
// current level.  If no square is available, does nothing.  If item
// has type ONONE, does nothing.
//
// Returns false if the item was created nearby (or not at all), true
// if no space was found and it was created elsewhere on the level.
void
createitem(int baseX, int baseY, struct Object item) {

    if (isnone(item)) { return; }

    int x = 0, y = 0;
    int radius = point_near(baseX, baseY, &x, &y, true, false);
    if (radius >= 0) {
        Map[x][y].obj = item;
    } else {
        say("You seen an object begin to form, then disappear.\n");
    }// if
}/* createitem*/



// Function to create a random item around coords x y.  Item is chosen
// randomly but is appropriate for cave level 'lev'.
void
create_rnd_item(int x, int y, int lev) {
    if (lev < 0 || lev > VBOTTOM) {
        return;     /* correct level? */
    }

    if (rnd(101) < 8) {
        create_rnd_item(x,y,lev);/* possibly more than one item */
    }

    createitem(x, y, newobject(lev));
}/* create_rnd_item*/


/*
 *  function to create a gem on a square near the player
 */
void
creategem () {
    int i,j;

    switch(rnd(4)) {
    case 1:
        i=ODIAMOND;
        j=50;
        break;
    case 2:
        i=ORUBY;
        j=40;
        break;
    case 3:
        i=OEMERALD;
        j=30;
        break;
    default:
        i=OSAPPHIRE;
        j=20;
        break;
    };
    createitem(UU.x, UU.y, obj(i, rnd(j) + (j / 10)));
}/* creategem */


// Mark the entire map seen/unseen
void
set_reveal(bool see) {
    for (int x=0; x < MAXX; x++) {
        for (int y=0; y < MAXY; y++) {
            if (see) {
                see_at(x, y);
            } else {
                forget_at(x, y);
            }// if .. else
        }/* for */
    }/* for */
}/* map_level*/
