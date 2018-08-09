// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Code for creating, destroying and managing in-game creatures.

#ifndef HDR_GUARD_MONSTER_H
#define HDR_GUARD_MONSTER_H

#include "player.h"


struct MonstTypeData {
    char            *name;
    char            mapchar;
    char            level;
    char            armorclass;
    char            damage;
    char            attack;
    char            intelligence;
    short           gold;
    short           hitpoints;
    long            experience;
    unsigned long   flags;
};

/* 
 *  Flags for monst structure
 */
enum MONSTER_FLAG {
    FL_NONE       = 0x00,  /* Nothing of interest                 */
    FL_GENOCIDED  = 0x01,  /* Monster has been genocided          */
    FL_DEMON      = 0x02,  /* Monster is a major demon. */
    FL_NOBEHEAD   = 0x04,  /* Monster cannot be beheaded by Vorpy */
    FL_NOPIT      = 0x08,  /* Won't move over a pit. */
    FL_SLOW       = 0x10,  /* Moves at half speed. */
    FL_INVISIBLE  = 0x20,  /* Non-demonic invisiblity. */
};

extern struct MonstTypeData MonType[];

struct Monster {
    short hitp;
    uint8_t id;
    int moved       :1;     // Has the monster here moved this turn?
    int awake       :1;     // Is the monster awake? (i.e. no stealth)
};

extern const struct Monster NullMon;

/* Big enum of monster IDs. */
enum MONSTER_ID {

#define MONSTER(id,sym,lv,ac,dmg,attack,int,gold,hp,exp,flags,longdesc) id,
#include "monster_list.h"
#undef MONSTER

    MAXCREATURE  = DEMONLORD1,   /* The max. number of non-demon monsters.*/
    NUM_MONSTERS = DEMONKING,    /* The number of monsters. */
};


/* Big enum of special attacks. */
enum SP_ATTACK {
    SA_NONE         = 0,
    SA_RUST         = 1,
    SA_FIRE         = 2,
    SA_BIGFIRE      = 3,
    SA_STING        = 4,
    SA_COLD         = 5,
    SA_DRAIN        = 6,
    SA_GUSHER       = 7,
    SA_STEALGOLD    = 8,
    SA_DISENCHANT   = 9,
    SA_TAILTHWACK   = 10,
    SA_CONFUSE      = 11,
    SA_MULTI        = 12,
    SA_PSIONICS     = 13,
    SA_STEAL        = 14,
    SA_BITE         = 15,
    SA_BIGBITE      = 16,

};

bool avoidspits(struct Monster mon);

void createmonster(int mon);
void createmonster_near(int mon, int pos_x, int pos_y);
int fillmonst(int what);
int fullhit(int xx);
const char *monname_at(int x, int y);
const char *monname(uint8_t id);
void hitmonster(int x, int y);
int hitm(int x, int y, int amt);
void hitplayer(int x, int y);
void dropgold(int amount);
void annihilate(void);
void genmonst(void);
int makemonst(int lev);
void randmonst(void);
void disappear(int x, int y);
bool cantsee(struct Monster mon);

// Return default monster hitpoints adjusted for challenge level
static inline short mon_hp(uint8_t id) {
    return min(0xFFFF,  ((6 + UU.challenge) * MonType[id].hitpoints + 1) / 6);
}

// Return the experience gained from killing a monster of type 'id'
// adjusted for the challenge level.
static inline long mon_exp(uint8_t id) {
    return max_l( 1, (7 * MonType[id].experience) / (7 + UU.challenge) + 1);
}// mon_hp

// Create a monster struct for the given ID
static inline struct Monster mk_mon(uint8_t id) {
    // Zero (no monster) works because mon_hp(0) == 0.
    return (struct Monster){mon_hp(id), id, 0, 0};
}// mk_mon


static inline bool ismon(struct Monster mon)  {return mon.id != NOMONST; }
static inline short maxhp(struct Monster mon) {return MonType[mon.id].hitpoints;}
static inline const char *monname_mon(struct Monster mon) {return monname(mon.id);}
static inline long monflags(struct Monster mon) {return MonType[mon.id].flags;}
static inline bool isdemon(struct Monster mon) {return !!(monflags(mon) & FL_DEMON);}
static inline bool isslow(struct Monster mon) {return !!(monflags(mon) & FL_SLOW);}

#endif
