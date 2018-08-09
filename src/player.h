// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Module for maintaining and managing the player state.  The player
// is stored in a global variable called 'UU'.

#ifndef HDR_GUARD_PLAYER_H
#define HDR_GUARD_PLAYER_H

#include "object.h"
#include "school.h"
#include "util.h"


enum CHAR_CLASS {
#   define CCLASS(id, desc) id,
#   include "char_classes.h"
#   undef CCLASS
    CC_COUNT
};
#define CC_LAST (CC_COUNT - 1);

/* This should actually be called "GENDER" since it affects how the
 * character is addressed by other characters rather than their
 * biology (gender is a social construct, after all), but "sex" is
 * used internally and is also shorter to type. */
enum SEX {FEMALE, MALE};

#define PLAYERNAME_MAX 40

struct Player {
    int x, y;              // Coordinates.
    int prev_x, prev_y;    // Previous coordinates; updated by moveplayer()
    enum CHAR_CLASS cclass;
    enum SEX sex;
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
    int gold;       // TODO: make long
    long experience;
    long level;     // Experience level
    long regen;
    long wclass;
    long ac;
    long bankaccount;
    char bankvisits;    // This is used to rotate bank slogans.
    long spellmax;
    long spells;
    long energy;
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
    long laughing;      /* not implemented */
    long drainstrength;
    long clumsiness;
    long infeeblement;
    long halfdam;
    long seeinvisible;
    long fillroom;

    long sphcast;   /* nz if an active sphere of annihilation */
    long wtw;       /* walk through walls */
    long strextra;  /* character strength due to objects or enchantments */
    long lifeprot;  /* life protection counter */

    long elvup; /* elevator up */
    long elvdown;   /* elevator down */

    long coked; /* timer for being coked out */
    long intelligence_pre_hammer;   // INT from before you picked up the hammer

    long outstanding_taxes;
};

enum ENCH_HOW {ENCH_SCROLL, ENCH_ALTAR};

#define IVENSIZE  26                    // max size of inventory
extern struct Player UU;                // global player object
extern struct Object Invent[IVENSIZE];  // global player inventory

const char *ccname(enum CHAR_CLASS cc);
enum CHAR_CLASS ccvalue(const char *name);

void init_new_player (enum CHAR_CLASS, enum SEX, int);

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
int take(struct Object thing, const char *);
void pickup(void);
int drop_object(int k);
void enchweapon(int how);
int inv_slots_free(void);
bool nearbymonst(void);
bool stealsomething(int x, int y);
bool emptyhanded(void);
void creategem(void);
void adjustcvalues(int itm, int arg);
int packweight(void);
bool moveplayer(DIRECTION dir, bool* success);
void adjusttime(long tim);
void raise_min(int min);
void add_to_base_stats(int val);

void enchantarmor(enum ENCH_HOW how);

bool graduated(struct Player *p);
void regen(void);

const char *levelDesc(int level);

    
/* Inlines: */
static inline const char *uuccname() {return ccname(UU.cclass);}
static inline bool lancedeath() {
    return UU.wield<IVENSIZE && UU.wield >= 0 && Invent[UU.wield].type==OLANCE;
}

static inline void moveplayer_back() {
    UU.x = UU.prev_x;
    UU.y = UU.prev_y;
}// moveplayer_back

// Pronoun helpers
static inline const char *his(enum SEX gender) {
    return gender == MALE ? "his" : "her";
}

#endif


