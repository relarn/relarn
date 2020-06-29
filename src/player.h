// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Module for maintaining and managing the player state.  The player
// is stored in a global variable called 'UU'.

#ifndef HDR_GUARD_PLAYER_H
#define HDR_GUARD_PLAYER_H

#include <stdint.h>

#include "object.h"
#include "school.h"
#include "util.h"
#include "gender.h"
#include "stat_type.h"
#include "sphere.h"
#include "cast.h"
#include "monster.h"
#include "char_ids.h"


#define PLAYERNAME_MAX 40

struct Player {
    // Player position on the current level.  (Current level is owned
    // by map.[ch] for Reasons.)
    int8_t x, y;                // Coordinates.
    //int8_t levelNum;            // Current dungeon level
    int16_t prev_x, prev_y;     // Prev coordinates; updated by moveplayer()

    // Player details
    enum CHAR_CLASS cclass;
    enum GENDER gender;
    enum GENDER spouse_gender;
    char name[PLAYERNAME_MAX];
    bool courses[EDU_LEVELS];   // Courses the player has taken
    int32_t outstanding_taxes;  // Taxed owed; set at game start
    uint16_t challenge;         // Difficulty.  I *hate* this.

    // Time
    int32_t gtime;              // Time (in turns)
    int32_t banktime;           // Last time the player entered the bank.
    bool created[OBJ_COUNT];    // Unique objects that have been created

    // Common stats.
    struct Stat strength;
    struct Stat intelligence;
    struct Stat wisdom;
    struct Stat constitution;
    struct Stat dexterity;
    struct Stat charisma;

    // The combination of armor class from worn stuff, defense from
    // protection effects and the occasional permanent base AC boost.
    // Labeled AC (armor class) for the player.
    struct Stat defense;

    // Hitpoint management
    int32_t hpmax;
    int32_t hp;
    int32_t regencounter;

    // Spell regeneration
    int32_t spellmax;
    int32_t spells;
    int32_t ecounter;

    // Experience
    int32_t experience;
    int16_t level;              // Experience level

    // Financial
    int64_t gold;               // 64 bits?  Look at Mx. Moneybags here!
    int64_t bankaccount;
    uint16_t bankvisits;        // This is used to rotate bank slogans.

    // Worn and wielded.
    int8_t wear;
    int8_t wield;
    int8_t shield;

    // Timed effects
    int32_t protectionTime;
    int32_t dexCount;
    int32_t strcount;
    int32_t blindCount;         // if blind
    int32_t confuse;            // if confused
    int32_t altpro;             // Protection granted by an altar
    int32_t hero;               // if hero
    int32_t charmcount;
    int32_t invisibility;       // if invisible
    int32_t cancellation;       // if cancel cast
    int32_t hasteSelf;          // if hasted
    int32_t aggravate;
    int32_t globe;              // Invulnerable Globe (aka Invulnerability)
    int32_t scaremonst;         // if scare cast
    int32_t awareness;          // if awareness cast (or otherwise in effect)
    int32_t holdmonst;
    int32_t timestop;
    int32_t hastemonst;
    int32_t spiritpro;
    int32_t undeadpro;
    int32_t giantstr;           // if giant strength
    int32_t fireresistance;
    int32_t stealth;
    int32_t seeinvisible;
    int32_t monster_detection;
    int32_t wtw;                // walk through walls
    int32_t itching;            // Curse: can't wear armor
    int32_t clumsiness;         // Curse: chance of dropping stuff
    int32_t halfdam;            // Curse: halves your damage
    int32_t coked;              // timer for being coked out

    struct EnlStat {            // Enlightenment; stored with radius
        uint8_t radius;
        uint8_t time;
    } enlightenment;

    // List of Spheres of Annihilation
    struct SphereList spherelist;
    //int8_t sphcast;           // Number of active Spheres of Annihilation

    // Information we're tracking about stuff
    bool known_obj[OBJ_COUNT];  // Array of identified objects
    bool spellknow[SPNUM];      // This is the array of spells currently known.
    bool banished[NUM_MONSTERS];// Banished monsters

    // Up- and down-elevators have been created. (We need to track
    // this because levels are created on demand.)
    bool has_up_elevator;
    bool has_down_elevator;

    // Other stuff
    int32_t lifeprot;           // life prot. counter; only decreases on death
    bool killedBigBad;          // Player killed DEMONKING; for the email.
    int16_t monstCount;         // Countdown 'til next random monster creation
    bool wizardMode;            // Enables certain debug features.

    // Cached value of has_a(OLARNEYE).  We do this because this value
    // gets checked a *lot* when drawing the screen so recalc() caches
    // it here at the start of each turn.  It is also set when
    // taking/losing the Eye.
    bool hasTheEyeOfLarn;

    // And we reserve some dead space at the end of the struct so that
    // we can add items without breaking savefile compatibility.  This
    // is an ugly hack, but then so is this game's save system.
    //int8_t unused[64];

};

enum ENCH_HOW {ENCH_SCROLL, ENCH_ALTAR};

#define IVENSIZE  26                    // max size of inventory
extern struct Player UU;                // global player object
extern struct Object Invent[IVENSIZE];  // global player inventory

const char *ccname(enum CHAR_CLASS cc);
enum CHAR_CLASS ccvalue(const char *name);

void init_new_player (enum CHAR_CLASS, enum GENDER, enum GENDER, int);

bool has_a(enum OBJECT_ID);
int index_of_first(enum OBJECT_ID type);

void drop_gold (int64_t amount);
struct Object inventremove(int index);

void raiselevel(void);
void loselevel(void);
void raiseexperience(int);
void loseexperience(int);
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
int packweight(void);
bool moveplayer(DIRECTION dir, bool* success);
void adjust_effect_timeouts(int32_t time, bool make_permanent);
void raise_min(uint16_t min);
void add_to_base_stats(int val);

void enchantarmor(enum ENCH_HOW how);

bool graduated(void);
void regen(void);
void recalc(void);
int32_t weaponclass(void);

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


static inline void banish_monster(enum MONSTER_ID id) {
    ASSERT(id < NUM_MONSTERS);
    UU.banished[id] = true;
}// banish_monster

static inline bool is_banished(enum MONSTER_ID id) {
    ASSERT(id < NUM_MONSTERS);
    return UU.banished[id];
}

static inline bool knows_obj(enum OBJECT_ID id) {
    ASSERT(id < OBJ_COUNT);

    return UU.known_obj[id];
}

static inline bool identify(enum OBJECT_ID id) {
    ASSERT(id < OBJ_COUNT);
    return UU.known_obj[id] = true;
}

static inline bool unidentify(enum OBJECT_ID id) {
    ASSERT(id < OBJ_COUNT);
    return UU.known_obj[id] = false;
}

static inline void identify_all() {
    for (int i = 0; i < OBJ_COUNT; i++) { identify(i); }
}

// Is the player low-level enough for the full lemmings experience?
//
// Lemmings are annoying and turn the game into a slog, PLUS it's
// an abusable way for low-level characters to safely level up.
//
// On the other hand, lemmings are part of the game's
// distinctive feel.
//
// So, we keep lemmings and they continue to reproduce like
// mad but only for the first few levels, enough to convey the
// experience (and let the player take a little advantage of
// them) but make it go away before it gets too old.
static inline bool annoying_lemmings() {
    return UU.level <= 4;
}


// Define functions for manipulating the common stats.  These are all
// wrappers around stat_type.h.

#define STAT_MIN 3
#define STAT_SUM_MIN 1

#define MK_STATFUN(attr)                                                   \
    static inline void attr##_init(int16_t base) {                         \
        stat_init(&UU.attr, base, STAT_MIN, STAT_SUM_MIN); }               \
    \
    static inline void attr##_adjust(int16_t val) {                        \
        stat_adjust(&UU.attr, val); }                                      \
    \
    static inline void attr##_adjust_min(int16_t val, int16_t minval) {   \
        stat_adjust_min(&UU.attr, val, minval); }                           \
    \
    static inline int16_t attr() { return stat_val(&UU.attr); }            \
    \
    static inline void attr##_reset_mod() { stat_reset_mod(&UU.attr); }     \
    \
    static inline void attr##_adjust_mod(int16_t val) {                    \
        stat_adjust_mod(&UU.attr, val); }


MK_STATFUN(strength);
MK_STATFUN(intelligence);
MK_STATFUN(wisdom);
MK_STATFUN(constitution);
MK_STATFUN(dexterity);
MK_STATFUN(charisma);

// Note: we don't use defense_init directly
MK_STATFUN(defense);


#undef MK_STATFUN
#undef STAT_MIN
#undef STAT_SUM_MIN
#endif
