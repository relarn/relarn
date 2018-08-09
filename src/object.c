// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "object.h"

#include "internal_assert.h"

#include "create.h"
#include "display.h"
#include "game.h"
#include "fortune.h"
#include "action.h"


const struct Object NullObj = {0, 0};

// Master list of object types:
struct ObjType Types[] = {
#define OBJECT(id, sym, mprice, mqty, mrust, weight, mod, mflags, mpdesc, mdesc) \
    {mpdesc mdesc, mdesc, sym, mprice, mqty, mrust, weight, mod, mflags,         \
     !((mflags) & (OA_SCROLL|OA_POTION))},
#   include "object_list.h"
#undef OBJECT
};


static void extendspells (void);


/* destroy object at present location */
void
udelobj() {
    Map[UU.x][UU.y].obj = NullObj;
    Map[UU.x][UU.y].know = false;
}/* udelobj*/


/* Return the flags of the type of obj. */
static unsigned long
objflags(struct Object obj) {
    ASSERT(obj.type <= MAX_OBJ);
    return Types[obj.type].flags;
}/* objflags*/


/* Test if object 'obj' is a potion. */
bool
ispotion(struct Object obj) {
    return !! (OA_POTION & objflags(obj));
}/* ispotion*/

/* Test if 'obj' is a drug. */
bool
isdrug(struct Object obj) {
    return !! (OA_DRUG & objflags(obj));
}/* ispotion*/


/* Test if object 'obj' is a scroll. */
bool
isscroll(struct Object obj) {
    return !! (OA_SCROLL & objflags(obj));
}/* isscroll*/

/* Test if object 'obj' is readable. */
bool
isreadable(struct Object obj) {
    ASSERT (obj.type < MAX_OBJ);
    return !! (OA_READABLE & objflags(obj));
}/* isscroll*/


/* Test if object 'obj' is identified. */
bool
isknown(struct Object obj) {
    return Types[obj.type].isKnown;
}/* isknown*/

/* Test if object 'obj' is a gem. */
bool
isgem(struct Object obj) {
    return !!(OA_GEM & objflags(obj));
}/* isgem*/

/* Test if obj can be bought in the store. */
bool
issellable(struct Object obj) {
    return !!(OA_CANSELL & objflags(obj));
}

bool
iswieldable(struct Object obj) {
    return !!(OA_WIELDABLE & objflags(obj));
}/* iswieldable*/

bool
iswearable(struct Object obj) {
    return !!(OA_WEARABLE & objflags(obj));
}/* iswieldable*/

/* Test if obj is the null object. */
bool
isnone(struct Object obj) {
    return obj.type == ONONE;
}/* isnone*/

/* Test if this is something a leprechaun would want to steal. */
bool
isshiny(struct Object obj) {
    return isgem(obj) || obj.type == OGOLDPILE;
}/* isshiny*/

/* Return the price the bank would give you for this piece. */
int
banksellvalue(struct Object obj) {
    if (obj.type == OLARNEYE) {
        return min(50000, 250000 - ((UU.gtime*7)/MOBUL)*MOBUL);
    }/* if */

    if (isgem(obj)) {
        return obj.iarg * 100;
    }/* if */

    return 0;
}/* banksellvalue*/


/* Returns a string containing the textual description of obj. Note:
 * the return value is static and so will be overwritten in the next
 * call to objname(). */
const char *
longobjname (struct Object obj) {
    static char desc[80];

    ASSERT (obj.type < OBJ_COUNT);

    if (obj.type == OGOLDPILE) {
        sprintf (desc, "%u gold pieces", (unsigned)obj.iarg);
    } else if ( (Types[obj.type].flags & (OA_WIELDABLE|OA_WEARABLE)) && obj.iarg != 0) {
        sprintf (desc, "%s %+d", objname(obj), (int)obj.iarg);
    } else {
        strncpy (desc, objname(obj), sizeof(desc));
    }/* if .. else*/

    return desc;
}/* objname */

/* Return the name of obj. */
const char *
objname(struct Object obj) {
    ASSERT(obj.type < OBJ_COUNT);
    return Types[obj.type].desc;
}/* objname*/

/* Return the short description for 'obj'*/
const char *
shortobjname(struct Object obj) {
    ASSERT(obj.type < OBJ_COUNT);
    return Types[obj.type].shortdesc;
}/* shortobjname*/

/* Return the name of obj as best as is known by the player. */
const char *
knownobjname(struct Object obj) {
    if (isknown(obj)) return objname(obj);

    if (ispotion(obj)) return "a magic potion";
    if (isscroll(obj)) return "a magic scroll";
    return objname(obj);
}/* knownobjname*/


/* Create a new object struct and return it (by value). */
struct Object
obj(enum OBJECT_ID id, unsigned arg) {
    struct Object obj;

    obj.type = id;
    obj.iarg = arg;

    ASSERT (id < OBJ_COUNT);    /* Ensure valid obj. id */
    ASSERT (obj.iarg == arg);   /* Ensure arg. fits in obj.iarg. */

    return obj;
}/* obj*/


/* Return a closed door, possibly with a trap.  Probabilities of trap
 * type are determined by 'risk'.  This is normally DTO_LOW with the
 * others only being needed for treasure room doors.. */
struct Object
door(enum DOORTRAP_RISK risk) {
    int trapnum = 0;
    enum DOOR_TRAP dt = DT_NONE;

    /* Original ULarn used numbers 6-9 to indicate the different traps
     * (7 and 8 were both shock).  Code in different places would pick
     * a value randomly from different ranges, thus controlling the
     * odds of a known trap and ocloseddoor() (originally part of
     * lookforobject()) would perform the appropriate action if the
     * iarg field was one of the trap values.
     *
     * All of this mess is now compacted here in order to preserve the
     * behaviour but now the iarg value is an element in DOOR_TRAP as
     * this makes much more sense. */

    /* First, compute a number from a range chosen by 'risk'. */
    switch (risk) {
    case DTO_LOW:       trapnum = rnd(30);      break;
    case DTO_MEDIUM:    trapnum = rnd(3)+6;     break;
    case DTO_HIGH:      trapnum = rnd(9);       break;
    }

    /* Now, use that number to select the appropriate trap ID. */
    switch (trapnum) {
    case 6:             dt = DT_AGGRAVATE;      break;
    case 7: case 8:     dt = DT_SHOCK;          break;
    case 9:             dt = DT_WEAKEN;         break;
    }

    /* And return the door. */
    return obj(OCLOSEDDOOR, dt);
}/* door*/


/*
  function to read a scroll
*/
void
read_scroll(struct Object scr) {
    int i,j;
    int typ = scr.type;

    ASSERT (isscroll(scr));

    Types[typ].isKnown = true;   /* Reading identifies the scroll. */

    say("You read a scroll of %s.\n", shortobjname(scr));

    switch(typ) {
    case OSENCHANTARM:
        enchantarmor(ENCH_SCROLL);
        return;

    case OSENCHANTWEAP:
        enchweapon(ENCH_SCROLL);
        return;

    case OSENLIGHTEN:
        nap(2000);
        enlighten(25, 7);
        return;

    case OSBLANK:
        return;

    case OSCREATEMONST:
        createmonster(makemonst(getlevel()+1));
        return;

    case OSCREATEITEM:
        something(UU.x, UU.y, getlevel());
        return;

    case OSAGGMONST:
        UU.aggravate+=800;
        return;

    case OSTIMEWARP:
        /*
         *  This code is slightly wrong in that, if UU.gtime is small and
         *  we can't go back by the required number of mobuls, it's
         *  still reported that we did.  I don't think this is
         *  critical -- dmr
         */
        i = (rnd(1000) - 850)/MOBUL;
        if (i==0) i=1;

        UU.gtime += MOBUL*i;
        if (UU.gtime < 0) UU.gtime = 0;

        say("You go %sward in time by %d mobul%s\n", (i<0)?"back":"for",
                (i<0)?-i:i, i==1?"":"s");

        adjusttime((long)(i * MOBUL));/* adjust time for time warping */
        return;

    case OSTELEPORT:
        teleport(0);
        return;

    case OSAWARENESS:
        UU.awareness += 1800;
        return;

    case OSHASTEMONST:
        UU.hastemonst += rnd(55)+12;
        say("You feel nervous.");
        return;

    case OSMONSTHEAL:
        for (i=0; i<MAXY; i++) {
            for (j=0; j<MAXX; j++) {
                uint8_t id = Map[j][i].mon.id;
                if (id) { Map[j][i].mon.hitp = mon_hp(id); }
            }// for
        }// for

        say("You feel uneasy.");
        return;

    case OSSPIRITPROT:
        UU.spiritpro += 300 + rnd(200);
        return;

    case OSUNDEADPROT:
        UU.undeadpro += 300 + rnd(200);
        return;

    case OSSTEALTH:
        UU.stealth += 250 + rnd(250);
        return;

    case OSMAGICMAP:
        for (i=0; i<MAXY; i++) {
            for (j=0; j<MAXX; j++) {
                Map[j][i].know = 1;
            }/* for */
        }/* for */
        nap(2000);
        update_display(true);
        return;

    case OSHOLDMONST:
        UU.holdmonst += 30;
        return;

    case OSGEMPERFECT:
        for (i=0; i<IVENSIZE; i++) {
            if (!isgem(Invent[i])) {
                continue;
            }

            j = Invent[i].iarg;
            j *= 2;
            if (j <= 0 && Invent[i].iarg)
                j=2550;
            Invent[i].iarg = j;
        }
        break;

    case OSSPELLEXT:
        extendspells();
        break;

    case OSIDENTIFY:
        for (i = 0; i < IVENSIZE; i++) {
            Types[Invent[i].type].isKnown = true;
        }/* for */
        break;

    case OSREMCURSE:
        removecurse();
        break;

    case OSANNIHILATE:
        annihilate();
        break;

    case OSPULVERIZE:
        godirect(22,150,"The ray hits the %s.",0,' ');
        break;

    case OSLIFEPROT:
        UU.lifeprot++;
        break;
    };
}/* read_scroll*/

/*
 *  Cure the player of curses.
 */
void
removecurse () {
    int i;
    static long *curse[] = {
        &UU.blindCount, &UU.confuse, &UU.aggravate, &UU.hastemonst,
        &UU.itching, &UU.laughing, &UU.drainstrength, &UU.clumsiness,
        &UU.infeeblement, &UU.halfdam,
        NULL
    };

    for (i=0; curse[i]; i++) {
        if (*(curse[i])) {
            *(curse[i]) = 1;
        }/* if */
    }/* for */
}/* removecurse */

static void
extendspells () {
    int i;
    static long *exten[] = {
        &UU.protectionTime, &UU.dexCount, &UU.strcount, &UU.charmcount,
        &UU.invisibility, &UU.cancellation, &UU.hasteSelf, &UU.globe,
        &UU.scaremonst, &UU.holdmonst, &UU.timestop,

        NULL
    };

    for (i=0; exten[i]; i++) {
        *(exten[i]) <<= 1;
    }/* for */
}/* extendspells */

/*
  function to read a book
*/
void
readbook(int booklev) {
    static int splev[] = {1,4,7,11,15,20,24,28,30,32,33,34,35,36,37,38};
    const size_t nlev = sizeof(splev)/sizeof(splev[0]);
    int spell, lev;

    ASSERT (booklev >= 0);

    if (booklev <= 3) {
        lev = splev[booklev];
        spell = rund((lev != 0) ? lev : 1);
    } else {
        if (booklev >= nlev) {
            booklev = nlev - 1;
        }/* if */

        lev = splev[booklev] - 9;
        spell = rnd( (lev != 0) ? lev : 1 ) + 9;
    }/* if .. else*/

    GS.spellknow[spell] = true;
    say("Spell \"%s\":  %s\n%s.\n",
        Spells[spell].code, Spells[spell].name, Spells[spell].desc);

    if (rnd(10)==4) {
        say("You feel more educated!\n");
        UU.intelligence++;
    }/* if */
}/* readbook*/


/*  routine to save program space   */
void
iopts() {
    say(", or (i) ignore it? ");
}/* iopts*/

void
ignore() {
    say("ignore.\n");
}/* ignore*/

void
closedoor() {
    int i;

    /* can't find objects is time is stopped*/
    if (UU.timestop)  return;

    showplayerarea();

    i=Map[UU.x][UU.y].obj.type;
    if (i != OOPENDOOR) {
        say("There is no open door here.\n");
        return;
    }
    say("The door closes.\n");
    udelobj();
    Map[UU.x][UU.y].obj = obj(OCLOSEDDOOR, 0);
    cancelLook(); /* So we won't be asked to open it */
}/* closedoor*/



/*
 * Routine to return a randomly selected object to be created. Returns
 * the new Object struct.
 *
 * Enter with the cave level.  The resulting argument is chosen based
 * on it.
 */

struct Object
newobject(int lev) {
    static char nobjtab[] = {
        0, OSENCHANTARM, OSENCHANTARM, OSENCHANTARM, OSENCHANTARM,
        OPSLEEP, OPSLEEP, OPSLEEP, OPSLEEP,     // gets replaced in the routine
        OGOLDPILE, OGOLDPILE, OGOLDPILE, OGOLDPILE,
        OBOOK, OBOOK, OBOOK, OBOOK,
        ODAGGER, ODAGGER, ODAGGER,
        OLEATHER, OLEATHER, OLEATHER,
        OREGENRING, OPROTRING, OENERGYRING, ODEXRING, OSTRRING,
        OSPEAR, OBELT, ORING, OSTUDLEATHER, OSHIELD, OFLAIL, OCHAIN,
        O2SWORD, OPLATE, OLONGSWORD
    };   /* 38 */

    int maxrnd = 32;
    int rndval;
    struct Object obj;

    if (getlevel() < 0 || getlevel() > VBOTTOM)
        return NullObj; /* correct level? */

    if (lev > 6) {
        maxrnd = 37;
    } else if (lev > 4) {
        maxrnd = 35;
    }/* if */

    rndval = rnd(maxrnd);
    obj.type = nobjtab[rndval];  /* the object type */
    switch (rndval) {
    case 1:
    case 2:
    case 3:
    case 4:
        obj = newscroll();
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        obj = newpotion();
        break;
    case 9:
    case 10:
    case 11:
    case 12:
        obj.type = OGOLDPILE;
        obj.iarg = rnd((lev + 1) * 10) + lev * 10 + 10;
        break;
    case 13:
    case 14:
    case 15:
    case 16:
        obj.type = OBOOK;
        obj.iarg = lev;
        break;
    case 17:
    case 18:
    case 19:
        obj = newdagger();
        break;
    case 20:
    case 21:
    case 22:
        obj = newleather();
        break;
    case 23:
    case 32:
    case 35:
        obj.iarg = rund(lev / 3 + 1);
        break;
    case 24:
    case 26:
        obj.iarg = rnd(lev / 4 + 1);
        break;
    case 25:
        obj.iarg = rund(lev / 4 + 1);
        break;
    case 27:
        obj.iarg = rnd(lev / 2 + 1);
        break;
    case 28:
        obj.iarg = rund(lev / 3 + 1);
        if (obj.iarg == 0)
            return NullObj;
        break;
    case 29:
    case 31:
        obj.iarg = rund(lev / 2 + 1);
        if (obj.iarg == 0)
            return NullObj;
        break;
    case 30:
    case 33:
        obj.iarg = rund(lev / 2 + 1);
        break;
    case 34:
        obj = newchain();
        break;
    case 36:
        obj = newplate();
        break;
    case 37:
        obj = newsword();
        break;
    }
    return obj;
}/* newobject*/



/* Return the sale value of 'obj' at the trading post.  -1 means the
 * store won't take it.*/
int
storesellvalue(struct Object obj) {
    int value;

    /* Gems aren't sold in the store but the trading post will buy
     * them from you (for far too little money). */
    if (isgem(obj)) {
        return obj.iarg * 20;
    }/* if */

    if (!issellable(obj)) {
        return -1;
    }/* if */

    /* Inventory price is in tens. */
    value = Types[obj.type].price;

    /* Increase from 10% to 20% unless it is enchantable and in poor
     * condition (i.e. has a negative enchantment). */
    if (!iswieldable(obj) || obj.iarg >= 0) {
        value *= 2;
    }

    /* In ULarn, the game won't let a Rambo sell the Lance of Death,
     * presumably on the principal that it's all he's got and selling
     * it would effectively end his game.  I dislike those sorts of
     * limitations and anyway, the rewritten UI makes it tricky to
     * implement, so instead, Rambo has the ability to get full price
     * for a LoD.  It'll get restocked and end up back in the store so
     * he can buy it back if need be. */
    if (UU.cclass == CCRAMBO && obj.type == OLANCE) {
        value *= 5;
    }/* if */

    /* If obj is enchantable, the enchantment level affects is sale
     * price. */
    if (iswieldable(obj)) {
        int izarg;

        for (izarg = obj.iarg; izarg; izarg--) {
            if (value >= 500000L) break;
            value = 14 * (67+value)/10;
        }/* for */
    }/* if */

    /* And insure that everything is worth at least one GP. */
    value = (value <= 0) ? 1 : value;

    return value;
}/* storesellvalue*/


/* Display the message indicating the player has eaten a fortune
 * cookie. */
void
show_cookie() {
    char *read, *ftext;

    if (UU.blindCount) {
        read = ".\nUnfortunately being blind, you can't read it.";
        ftext = "";
    } else {
        read = " that says:\n";
        ftext = fortune();
    }/* if .. else*/

    say("The cookie was delicious.  Inside you find a scrap of paper%s%s\n",
            read, ftext);
}/* show_cookie*/