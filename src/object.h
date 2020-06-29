// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Code for creating and managing in-game objects (i.e. weapons,
// armor, gems, etc.)

#ifndef HDR_GUARD_OBJECT_H
#define HDR_GUARD_OBJECT_H

#include "internal_assert.h"

#include <stdbool.h>
#include <stdint.h>

/*
   The type used to represent objects on the map and in inventory.
*/
#define MAX_IARG ((int)((1u << 23) - 1))
struct Object {
    uint8_t type;              /* Index of type in Types[]. */
    int iarg:24;
};


/*
  The Master List of Objects
*/

/* The set of attributes an object may have, ORed together.  Some
 * combinations don't make sense, so don't make those. */
enum OBJECT_ATTR {
    OA_NONE         = 0x0,
    OA_MOVABLE      = 0x1,      /* Can be picked up. */
    OA_CANSELL      = 0x2,      /* Can be sold in the DND store. */
    OA_WEARABLE     = 0x4,      /* Can be worn. (I.e. is armor.) */
    OA_WIELDABLE    = 0x8,      /* Can be wielded as a weapon. */
    OA_READABLE     = 0x10,     /* Can be read. */
    OA_SCROLL       = 0x20,     /* Is a scroll. */
    OA_GEM          = 0x40,     /* Is a gem. */
    OA_POTION       = 0x80,     /* Is a potion */
    OA_EDIBLE       = 0x100,    /* Can be eaten? Only true for cookies. */
    OA_CHARM        = 0x200,    /* Intended to be carried. */
    OA_BANKBUYS     = 0x400,    /* Can be sold to the bank. */
    OA_DRUG         = 0x800,    /* Sold exclusively by Dealer McDope. */
    OA_CANRUST      = 0x1000,   /* Can be rusted to -10 and destroyed. */
};

/* The list of object IDs. */
enum OBJECT_ID {
#define OBJECT(id, sym, price, qty, rust, weight, mod, flags, pdesc, desc) id,
#   include "object_list.h"
#undef OBJECT
    OBJ_COUNT,
    OBJ_CONCRETE_COUNT = OUNSEEN,
};

/* Keep these synched with object_list.h */
#define POTION_FIRST OPSLEEP
#define POTION_LAST OPSEEINVIS
#define NUM_POTIONS (POTION_LAST - POTION_FIRST)

#define SCROLL_FIRST OSENCHANTARM
#define SCROLL_LAST OSLIFEPROT
#define NUM_SCROLLS (SCROLL_LAST - SCROLL_FIRST)

// The null object; used to indicate nothing is in this cell (well,
// except for the floor).
#define NULL_OBJ ((struct Object) {ONONE, 0})

// Sentinal value to be used ONLY in MapSquare.recalled to indicate
// that the current location has not been explored.  I.e. more null
// object than NULL_OBJ.
#define UNSEEN_OBJ ((struct Object){OUNSEEN, 0})


enum DOOR_TRAP {
    DT_NONE,            /* No trap. */
    DT_AGGRAVATE,       /* Aggravate monsters. */
    DT_SHOCK,           /* Electric shock */
    DT_WEAKEN,          /* Lose one point of strength. */
};

enum DOORTRAP_RISK {
    DTO_LOW,            /* 4 in 30 */
    DTO_MEDIUM,         /* 1 in 3 aggravate, else zap. */
    DTO_HIGH,           /* 4 in 9 */
};


struct ObjType {
    const char *desc;
    const char *shortdesc;
    char symbol;
    long price;
    int storeQty;
    int maxrust;
    int weight;
    int mod;
    unsigned long flags;
    bool isKnownByDefault;
};
extern const struct ObjType Types[];

struct Object obj(enum OBJECT_ID id, unsigned arg);
struct Object door(enum DOORTRAP_RISK risk);

const char *objname(struct Object obj);
const char *shortobjname(struct Object obj);
const char *longobjname (struct Object obj);
const char *knownobjname(struct Object obj);

int storesellvalue(struct Object obj);
int banksellvalue(struct Object obj);


void quaffpotion(struct Object pot);
void read_scroll(struct Object scr);
void removecurse(void);
void readbook(int arg);
void iopts(void);
void ignore(void);
void closedoor(void);
struct Object newobject(int lev);
void show_cookie(void);


// Return the flags of the type of obj.
static inline unsigned long objflags(struct Object obj) {
    ASSERT(obj.type < OBJ_COUNT);
    return Types[obj.type].flags;
}

static inline bool ispotion(struct Object obj) {
    return !! (OA_POTION & objflags(obj));
}

// Test if 'obj' is a drug (from Dealer McDope).
static inline bool isdrug(struct Object obj) {
    return !! (OA_DRUG & objflags(obj));
}


static inline bool isscroll(struct Object obj) {
    return !! (OA_SCROLL & objflags(obj));
}

// Test if object can be read like a book (or scroll)
static inline bool isreadable(struct Object obj) {
    ASSERT (obj.type < OBJ_COUNT);
    return !! (OA_READABLE & objflags(obj));
}

static inline bool isgem(struct Object obj) {
    return !!(OA_GEM & objflags(obj));
}

// Test if obj can be bought in the store.
static inline bool issellable(struct Object obj) {
    return !!(OA_CANSELL & objflags(obj));
}

static inline bool iswieldable(struct Object obj) {
    return !!(OA_WIELDABLE & objflags(obj));
}

static inline bool iswearable(struct Object obj) {
    return !!(OA_WEARABLE & objflags(obj));
}

// Test if obj is the null object.
static inline bool isnone(struct Object obj) {
    return obj.type == ONONE;
}

// Test if this is something a leprechaun would want to steal.
static inline bool isshiny(struct Object obj) {
    return isgem(obj) || obj.type == OGOLDPILE;
}

// Test if this object blocks line-of-sight.
static inline bool isopaque(struct Object obj) {
    return obj.type == OWALL || obj.type == OCLOSEDDOOR;
}


// Test if the object can be dulled below 0.
static inline bool canrust(struct Object obj) {
    return !! (OA_CANRUST & objflags(obj));
}



#endif
