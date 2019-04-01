// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Module for maintaining and managing the player state.  The player
// is stored in a global variable called 'UU'.

#ifndef HDR_GUARD_PLAYER_H
#define HDR_GUARD_PLAYER_H

#include "object.h"
#include "school.h"
#include "util.h"
#include "gender.h"

enum CHAR_CLASS {
#   define CCLASS(id, desc) id,
#   include "char_classes.h"
#   undef CCLASS
    CC_COUNT
};
#define CC_LAST (CC_COUNT - 1);


#define PLAYERNAME_MAX 40

struct Player {
    int x, y;              // Coordinates.
    int prev_x, prev_y;    // Previous coordinates; updated by moveplayer()
    enum CHAR_CLASS cclass;
    enum GENDER gender;
    enum GENDER spouse_gender;
    char name[PLAYERNAME_MAX];

    long gtime;
    long banktime;      /* Last time the player entered the bank. */
    bool courses[EDU_LEVELS];
    bool created[OBJ_COUNT];    /* If obj is unique, a matching true
                                 * means it's been created. */
    long strength;  /* characters physical strength not due to objects */
    long intelligence;
    long wisdom;
    long constitution;
    long dexterity;
    long charisma;

    long hpmax;
    long hp;
    long gold;
    long experience;
    int  level;         // Experience level
    long bankaccount;
    char bankvisits;    // This is used to rotate bank slogans.
    long spellmax;
    long spells;
    long ecounter;
    long moredefenses;
    long wear;
    long protectionTime;
    long wield;
    long amulet;        /* if have amulet of invisibility */ // NOT REF'd
    long regencounter;
    long moreDmg;
    long dexCount;
    long strcount;
    long blindCount;        /* if blind */
    long confuse;       /* if confused */
    long altpro;
    long hero;          /* if hero  */
    long charmcount;
    long invisibility;      /* if invisible */
    long cancellation;      /* if cancel cast */
    long hasteSelf;     /* if hasted */
    long eyeOfLarn;     /* if have eye */
    long aggravate;
    long globe;
    long teleflag;      /* if been teleported */
    long slaying;       /* if have orb of dragon slaying */
    long negatespirit;      /* if negate spirit */
    long scaremonst;        /* if scare cast */
    long awareness;     /* if awareness cast */
    bool orb;
    long holdmonst;
    long timestop;
    long hastemonst;
    long cube_of_undead;    /* if have cube */
    long giantstr;      /* if giant strength */
    long fireresistance;
    long bessmann;      /* flag for hammer */
    long notheft;
    unsigned challenge;

    long spiritpro;
    long undeadpro;
    long shield;
    long stealth;
    long itching;
    long monster_detection;
    long drainstrength;
    long clumsiness;
    long infeeblement;
    long halfdam;
    long seeinvisible;
    long fillroom;

    struct EnlStat {
        uint8_t radius;
        uint8_t time;
    } enlightenment;

    long sphcast;   /* nz if an active sphere of annihilation */
    long wtw;       /* walk through walls */
    long strextra;  /* character strength due to objects or enchantments */
    long lifeprot;  /* life protection counter */

    // Up- and down-elevators have been created. (We need to track
    // this because levels are created on demand.)
    bool has_up_elevator;
    bool has_down_elevator;

    long coked; /* timer for being coked out */
    long intelligence_pre_hammer;   // INT from before you picked up the hammer

    long outstanding_taxes;

    // These are recomputed each turn by recalc() based on inventory
    // and other properties.
    long cached_ac;
    long cached_regen_rate;
    long cached_wc;
    long cached_spell_regen_boost;
};

enum ENCH_HOW {ENCH_SCROLL, ENCH_ALTAR};

#define IVENSIZE  26                    // max size of inventory
extern struct Player UU;                // global player object
extern struct Object Invent[IVENSIZE];  // global player inventory

const char *ccname(enum CHAR_CLASS cc);
enum CHAR_CLASS ccvalue(const char *name);

void init_new_player (enum CHAR_CLASS, enum GENDER, enum GENDER, int);

bool has_a(enum OBJECT_ID);

void drop_gold (unsigned long amount);
struct Object inventremove(int index);

void raiselevel(void);
void loselevel(void);
void raiseexperience(long x);
void loseexperience(long x);
void losehp(int x, int cause);
void losemhp(int x);
void raisehp(int x);
void raisemhp(int x);
void raisespells(int x);
void raisemspells(int x);
void losespells(int x);
void losemspells(int x);
void positionplayer(void);
void quit(void);
bool take(struct Object thing, const char *);
void pickup(void);
int drop_object(int k);
void enchweapon(int how);
int inv_slots_free(void);
bool nearbymonst(void);
bool stealsomething(int x, int y);
bool emptyhanded(void);
void adjustcvalues(int itm, int arg);
int packweight(void);
bool moveplayer(DIRECTION dir, bool* success);
void adjusttime(long time);
void raise_min(int min);
void add_to_base_stats(int val);

void enchantarmor(enum ENCH_HOW how);

bool graduated(struct Player *p);
void regen(void);
void recalc(void);

const char *levelDesc(int level);


/* Inlines: */
static inline const char *uuccname() {return ccname(UU.cclass);}

static inline bool wielding(enum OBJECT_ID id) {
    return UU.wield < IVENSIZE && UU.wield >= 0 &&
        Invent[UU.wield].type == id;
}// wielding

static inline void moveplayer_back() {
    UU.x = UU.prev_x;
    UU.y = UU.prev_y;
}// moveplayer_back

static inline void enlighten(uint8_t radius, uint8_t time) {
    UU.enlightenment = (struct EnlStat) {radius, time};
}

static inline int inv_limit() {
    return min(IVENSIZE, (15 + UU.level) >> 1);
}// inv_limit


#endif
