// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Code for creating, destroying and managing in-game creatures.

#ifndef HDR_GUARD_MONSTER_H
#define HDR_GUARD_MONSTER_H

#include <stdint.h>

#include "internal_assert.h"


struct MonstTypeData {
    char            *name;
    char            mapchar;
    uint8_t         level;
    int8_t          armorclass;     // lower is better, unlike the player's AC
    int8_t          damage;
    uint8_t         attack;
    uint8_t         intelligence;
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
    //FL_UNISED   = 0x01,  /* Removed; available for reuse. */
    FL_DEMON      = 0x02,  /* Monster is a major demon. */
    FL_NOBEHEAD   = 0x04,  /* Monster cannot be beheaded by Vorpy */
    FL_NOPIT      = 0x08,  /* Won't move over a pit. */
    FL_SLOW       = 0x10,  /* Moves at half speed. */
    FL_INVISIBLE  = 0x20,  /* Non-demonic invisiblity. */
    FL_DRAGON     = 0x40,  /* It's a dragon for Orb purposes. */
    FL_DULLS      = 0x80,  /* It can dull weapons when attacked. */
};

extern const struct MonstTypeData MonType[];

struct Monster {
    int16_t hitp;
    uint8_t id;             // Monster ID; 0 means no monster.
    int moved       :1;     // Has the monster here moved this turn?
    int awake       :1;     // Will it notice a stealthy player?
};

/* Big enum of monster IDs. */
enum MONSTER_ID {

#define MONSTER(id,sym,lv,ac,dmg,attack,int,gold,hp,exp,flags,longdesc) id,
#include "monster_list.h"
#undef MONSTER

    MAXCREATURE  = DEMONLORD1,   /* The max. number of non-demon monsters.*/
    LAST_MONSTER = DEMONKING,    /* Index of the last monster */
    NUM_MONSTERS                 /* The number of monsters. */
};


#define NULL_MON ((struct Monster) {0, 0, 0, 0})



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

void createmonster(enum MONSTER_ID mon);
void createmonster_near(enum MONSTER_ID mon, int pos_x, int pos_y);
int fillmonst(int what);
int fullhit(int xx);
const char *monname_at(int x, int y);
const char *monname(uint8_t id);
void hit_mon_melee(int x, int y);
int hitm(int x, int y, int amt, bool is_melee);
void hitplayer(int x, int y);
void dropgold(int amount);
void annihilate(void);
void banmonst(void);
int makemonst(int lev);
void randmonst(void);
void disappear(int x, int y);
bool cantsee(struct Monster mon);
short mon_hp(uint8_t id);
int mon_exp(uint8_t id);


// Create a monster struct for the given ID
static inline struct Monster mk_mon(uint8_t id) {
    // Zero (no monster) works because mon_hp(0) == 0.
    return (struct Monster){mon_hp(id), id, 0, 0};
}// mk_mon




static inline bool ismon(struct Monster mon)  {return mon.id != NOMONST; }
static inline short maxhp(struct Monster mon) {return MonType[mon.id].hitpoints;}
static inline short mon_ac(struct Monster mon) {return MonType[mon.id].armorclass;}
static inline const char *monname_mon(struct Monster mon) {return monname(mon.id);}
static inline char monchar(enum MONSTER_ID id) {return MonType[id].mapchar; }
static inline char monchar_mon(struct Monster mon) {return monchar(mon.id);}
static inline long monflags(struct Monster mon) {return MonType[mon.id].flags;}

static inline bool isdemon(struct Monster mon) {
    return !!(monflags(mon) & FL_DEMON);
}

static inline bool isslow(struct Monster mon) {
    return !!(monflags(mon) & FL_SLOW);
}

static inline bool isdragon(struct Monster mon) {
    return !!(monflags(mon) & FL_DRAGON);
}

static inline bool isdulling(struct Monster mon) {
    return !!(monflags(mon) & FL_DULLS);
}


#endif
