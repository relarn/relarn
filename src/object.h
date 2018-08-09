// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Code for creating and managing in-game objects (i.e. weapons,
// armor, gems, etc.)

#ifndef HDR_GUARD_OBJECT_H
#define HDR_GUARD_OBJECT_H

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

extern const struct Object NullObj;


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
    OA_DRUG       = 0x800,    /* Sold exclusively by Dealer McDope. */
};

/* The list of object IDs. */
enum OBJECT_ID {
#define OBJECT(id, sym, price, qty, rust, weight, mod, flags, pdesc, desc) id,
#   include "object_list.h"
#undef OBJECT
    OBJ_COUNT,    
};
#define MAX_OBJ (OBJ_COUNT - 1)

/* Keep these synched with object_list.h */
#define POTION_FIRST OPSLEEP
#define POTION_LAST OPSEEINVIS
#define NUM_POTIONS (POTION_LAST - POTION_FIRST)

#define SCROLL_FIRST OSENCHANTARM
#define SCROLL_LAST OSLIFEPROT
#define NUM_SCROLLS (SCROLL_LAST - SCROLL_FIRST)

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
    bool isKnown; 
};
extern struct ObjType Types[];

struct Object obj(enum OBJECT_ID id, unsigned arg);
struct Object door(enum DOORTRAP_RISK risk);

const char *objname(struct Object obj);
const char *shortobjname(struct Object obj);
const char *longobjname (struct Object obj);
const char *knownobjname(struct Object obj);

int storesellvalue(struct Object obj);
int banksellvalue(struct Object obj);

bool ispotion(struct Object obj);
bool isdrug(struct Object obj);
bool isscroll(struct Object obj);
bool isknown(struct Object obj);
bool isgem(struct Object obj);
bool issellable(struct Object obj);
bool isreadable(struct Object obj);
bool iswieldable(struct Object obj);
bool iswearable(struct Object obj);
bool isnone(struct Object obj);
bool isshiny(struct Object obj);

bool lookforobject(void);
void quaffpotion(struct Object pot);
void read_scroll(struct Object scr);
void removecurse(void);
void readbook(int arg);
void iopts(void);
void ignore(void);
void closedoor(void);
void udelobj(void);
struct Object newobject(int lev);
void show_cookie(void);

#endif
