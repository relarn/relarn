// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "internal_assert.h"

#include "display.h"
#include "score_file.h"
#include "show.h"
#include "game.h"
#include "settings.h"
#include "os.h"
#include "look.h"
#include "ui.h"

#include "player.h"

#include <limits.h>


// The global player state object.
struct Player UU;

/* Global player inventory. */
struct Object Invent[IVENSIZE];


// Various offsets for the stat mods performed by recalc()
enum STAT_MODS {
    // Carried
    SM_HAMMER       = 10,
    SM_SW_SLASHING  = 5,
    SM_SLAYER       = 10,
    SM_STAFF        = 10,

    // Drugs
    SM_COKED        = 33,
    SM_DEX          = 3,

    // Potions
    SM_HEROISM      = 10,
    SM_GIANTSTR     = 20,

    // Spells
    SM_STRENGTH     = 3,
    SM_PROTECT      = 2,
    SM_INVULN       = 10,

    // Other effects
    SM_ALTPRO       = 3,
};


static void item_loss_action(struct Object thing);
static void item_gain_action (struct Object thing);


/* Return a (read-only) string describing the character class given by
 * 'cc'. */
const char *
ccname(enum CHAR_CLASS cc) {
    static char *ccnames[] = {
#       define CCLASS(id, desc) desc,
#       include "char_classes.h"
#       undef CCLASS
        ""
    };

    ASSERT(cc >= 0 && cc < CC_COUNT);
    return ccnames[cc];
}/* ccname*/


/* Given a string describing a character class, return the matching CC
 * enum or CCNONE if the name is unknown.  This is the reverse of
 * ccname() and inputs must come from that list. */
enum CHAR_CLASS
ccvalue(const char *name) {
    static char *ccnames[] = {
#       define CCLASS(id, desc) desc,
#       include "char_classes.h"
#       undef CCLASS
        ""
    };
    int n;

    for (n = 0; *ccnames[n]; n++) {
        if (strcmp(name, ccnames[n]) == 0) {
            return (enum CHAR_CLASS)n;
        }
    }/* for */

    return CCNONE;
}/* ccname*/


static struct Object
random_potion() {
    return obj(rund(6) + POTION_FIRST, 0);
}

static struct Object
random_scroll() {
    return obj(SCROLL_FIRST + rund(6), 0);
}

// Initialize the player to a specific character class.
static void
init_cc_specific (enum CHAR_CLASS cc) {
    switch(cc) {
    case CCOGRE:
        UU.spellknow[CMMISSILE] = true;  /* mle */
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=16;
        strength_init(18);   /* strength */
        intelligence_init(4);    /* intelligence */
        wisdom_init(6);  /* wisdom */
        constitution_init(16);   /* constitution */
        dexterity_init(6);   /* dexterity */
        charisma_init(4);    /* charisma */
        Invent[0] = random_potion();
        Invent[1] = random_potion();
        break;

    case CCWIZARD:
        UU.spellknow[CMMISSILE] = true;  /* mle */
        UU.spellknow[CCHARM] = true;  /* chm */
        UU.spellmax=UU.spells=2;
        UU.hpmax=UU.hp=8;
        strength_init(8);    /* strength */
        intelligence_init(16);   /* intelligence */
        wisdom_init(16); /* wisdom */
        constitution_init(6);    /* constitution */
        dexterity_init(6);   /* dexterity */
        charisma_init(8);    /* charisma */
        Invent[0] = obj(OPTREASURE, 0); /* potion of treasure detection */
        Invent[1] = random_scroll();
        Invent[2] = random_scroll();
        break;

    case CCKLINGON:
        UU.spellknow[CSSPEAR] = true;  /* ssp */
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=14;
        strength_init(14);   /* strength */
        intelligence_init(12);   /* intelligence */
        wisdom_init(4);  /* wisdom */
        constitution_init(12);   /* constitution */
        dexterity_init(8);   /* dexterity */
        charisma_init(3);    /* charisma */
        Invent[0] = obj(OSTUDLEATHER, 0);
        Invent[1] = random_potion();
        UU.wear = 0;
        break;

    case CCELF:
        UU.spellknow[CPROT] = true;
        UU.spells=1;
        UU.spellmax=2;
        UU.hpmax=UU.hp=8;
        strength_init(8);    /* strength */
        intelligence_init(14);   /* intelligence */
        wisdom_init(12); /* wisdom */
        constitution_init(8);    /* constitution */
        dexterity_init(8);   /* dexterity */
        charisma_init(14);   /* charisma */
        Invent[0] = obj(OLEATHER, 0);
        Invent[1] = random_scroll();
        UU.wear=0;
        break;

    case CCROGUE:
        UU.spellknow[CMMISSILE] = true;
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=12;
        strength_init(8);    /* strength */
        intelligence_init(12);   /* intelligence */
        wisdom_init(8);  /* wisdom */
        constitution_init(10);   /* constitution */
        dexterity_init(14);  /* dexterity */
        charisma_init(6);    /* charisma */

        Invent[0] = obj (OLEATHER, 0);
        Invent[1] = obj (ODAGGER, 0);
        Invent[2] = obj (OSSTEALTH, 0);  /* stealth */

        UU.wear=0;
        UU.wield=1;
        break;

    case CCGEEK:
        UU.spellknow[CPROT] = true;
        UU.spellknow[CMMISSILE] = true;
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=10;
        strength_init(12);   /* strength */
        intelligence_init(12);   /* intelligence */
        wisdom_init(12); /* wisdom */
        constitution_init(12);   /* constitution */
        dexterity_init(12);  /* dexterity */
        charisma_init(12);   /* charisma */

        Invent[0] = obj(OLEATHER, 0);
        Invent[1] = obj(ODAGGER, 0);

        UU.wear=0;
        UU.wield=1;
        break;

    case CCDWARF:
        UU.spellknow[CPROT] = true;
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=12;
        strength_init(16);   /* strength */
        intelligence_init(6);    /* intelligence */
        wisdom_init(8);  /* wisdom */
        constitution_init(16);   /* constitution */
        dexterity_init(4);   /* dexterity */
        charisma_init(4);    /* charisma */

        Invent[0] = obj(OSPEAR, 0);

        UU.wield=0;
        break;

    case CCRAMBO:
        UU.spellmax=UU.spells=0;
        UU.hpmax=UU.hp=1;
        strength_init(3);    /* strength */
        intelligence_init(3);    /* intelligence */
        wisdom_init(3);  /* wisdom */
        constitution_init(3);    /* constitution */
        dexterity_init(3);   /* dexterity */
        charisma_init(3);    /* charisma */
        Invent[0] = obj(OLANCE, 0);

        UU.wield=0;
        break;

    default:
        // Should be unreachable
        FAIL("Unvalid character class.");
        break;
    } /* end switch */
}/* init_cc_specific */


// Initialize the player for the start of a new game.
void
init_new_player (enum CHAR_CLASS cc,
                 enum GENDER gender,
                 enum GENDER spouse_gender,
                 int difficulty) {
    int n;

    UU.cclass = cc;
    UU.gender = gender;
    UU.spouse_gender = spouse_gender;
    UU.challenge = difficulty;
    UU.monstCount = 80;

    zstrncpy(UU.name, GameSettings.name, sizeof(UU.name));

    UU.level = 1;     /*  player starts at level one  */
    UU.regencounter = 16;
    UU.ecounter = 96;     /*start regeneration correctly*/
    UU.shield = UU.wear = UU.wield = -1;

    for (n = 0; n < IVENSIZE; n++) {
        Invent[n] = obj(ONONE, 0);
    }/* for */

    init_cc_specific(cc);

    stat_init(&UU.defense, 0, 0, -100);

    UU.x = rnd(MAXX-2);
    UU.y = rnd(MAXY-2);
    UU.gtime = 0;    /*  time clock starts at zero   */

    // If the player previously won a game, they will owe taxes
    UU.outstanding_taxes = get_taxes_owed();

    // Identify everything that's known by default.
    for (int n = 0; n < OBJ_COUNT; n++) {
        if (Types[n].isKnownByDefault) {
            identify(n);
        }
    }// for

    /* Identify a potion of Cure Dianthroritis for free. */
    identify(OPCUREDIANTH);
}/* init_new_player */


// Return the index in inventory of the first object of type 'type' or
// a negative value if it's not present.
int
index_of_first(enum OBJECT_ID type) {
    for (int n = 0; n < IVENSIZE; n++) {
        if (Invent[n].type == type) {
            return n;
        }
    }/* for */

    return -1;
}// index_of_first

bool
has_a(enum OBJECT_ID type) {
    return index_of_first(type) >= 0;
}/* has_a*/


// Remove hitpoints; player dies if hitpoints reach 0.
void
losehp (int x, int cause) {
    if ((UU.hp -= x) <= 0) {
        headsup();
        say("Alas, you have died.\n");
        game_over_probably(cause);
    }
}/* losehp */


/* subroutine to remove max # hit points */
void
losemhp (int x) {
    UU.hp = max_l(1, UU.hp - x);
    UU.hpmax = max_l(1, UU.hpmax - x);
}/* losemhp */


// Raise hitpoints by x, capped my UU.hpmax
void
raisehp (int x) {
    UU.hp += x;
    if (UU.hp > UU.hpmax) {
        UU.hp = UU.hpmax;
    }
}/* raisehp */

// Increase maximum hitpoints (and actual hitpoints--bonus!) by x.
void
raisemhp (int x) {
    UU.hpmax += x;
    UU.hp += x;
}/* raisemhp */


// Raise spells by x, capped to UU.spellmax.
void
raisespells (int x) {
    UU.spells += x;
    if (UU.spells > UU.spellmax) {
        UU.spells = UU.spellmax;
    }// if
}/* raisespells */


// Raise maximum spells by x.
void
raisemspells (int x) {
    UU.spellmax+=x;
    UU.spells+=x;
}/* raisemspells */


// Lose spells
void
losespells (int x) {
    if ((UU.spells -= x) < 0)
        UU.spells=0;
}/* losespells */

/* subroutine to lose spell maximum */
void
losemspells (int x) {
    if ((UU.spellmax -= x) < 0)
        UU.spellmax=0;
    if ((UU.spells -= x) < 0)
        UU.spells=0;
}/* losemspells */


/*
 *  positionplayer()
 *
 *  function to be sure player is not in a wall
 */
void
positionplayer () {
    int try = 3;

    while ((at(UU.x, UU.y)->obj.type || at(UU.x, UU.y)->mon.id) && try) {
        if (++UU.x >= MAXX-1) {
            UU.x = 1;
            if (++UU.y >= MAXY-1) {
                UU.y = 1;
                --try;
            }/* if */
        }/* if */
    }/* while */

    if (try == 0) {
        // We shouldn't get here.
        say("That felt WIERD!\n");
    }
}/* positionplayer */


/*
 * subroutine to move the player from one cell to another
 *
 * returns false if can't move in that direction or hit a monster or on an object
 * else returns true.
 *
 * '*success' is set to true if the action succeeded, (i.e. the turn
 * can continue) and false if not (i.e. the move is impossible so no
 * action took place).  'success' must be a valid pointer.
 *
 * If 'success' is false, the result will also be false but the
 * reverse is not always true.  (E.g. hitting a monster doesn't move
 * the player but does consume a turn.)
 */

bool
moveplayer (DIRECTION dir, bool* success) {
    *success = true;

    if (dir == DIR_CANCEL) {
        *success = false;  /* CANCEL never succeeds 'cause it's invalid. */
        return false;
    }/* if */

    if (UU.confuse && UU.level < rnd(30) && dir != DIR_STAY) {
        // If confused, any dir.
        dir = randdir();
    }/* if */

    int8_t new_x, new_y;
    adjpoint(UU.x, UU.y, dir, &new_x, &new_y);

    if (new_x < 0 || new_x >= MAXX || new_y < 0 || new_y >= MAXY) {
        *success = false;
        return false;
    }

    uint8_t thing = at(new_x, new_y)->obj.type;

    /*  hit a wall (or closed door while time has stopped) */
    if ((thing == OWALL && UU.wtw == 0) ||
        (UU.timestop != 0 && thing == OCLOSEDDOOR))
    {
        bool foundit = false;

        /* If blind and the destination is unknown, reveal it but
         * spend a turn doing it. */
        if (UU.blindCount > 0 && !known_at(new_x, new_y)) {
            see_and_update_at(new_x, new_y);
            foundit = true;
        }/* if */

        *success = foundit;
        return false;
    }// if

    if (at(new_x, new_y)->mon.id > 0) {
        hit_mon_melee(new_x, new_y);
        return false;
    } /* hit a monster*/

    UU.prev_x = UU.x;
    UU.prev_y = UU.y;
    UU.x = new_x;
    UU.y = new_y;

    if (thing && thing != OTRAPARROWIV && thing != OIVTELETRAP &&
        thing != OIVDARTRAP && thing != OIVTRAPDOOR)
    {
        return false;
    } else {
        return true;
    }
}/* moveplayer */


/* Return the modifier field for the item at inventory slot 'slot' or
 * 0 if none is set. */
static int
statfor(int slot) {
    struct Object obj;

    if (slot < 0) return 0;
    ASSERT(slot < IVENSIZE);

    obj = Invent[slot];
    if (!obj.type) return 0;    /* Probably not needed. */

    return Types[obj.type].mod + obj.iarg;
}/* statfor*/


/* Return the modification value provided by carrying an object of
 * type 'oid'.  This defaults to 1 + its iarg with a couple of special
 * cases.  It is up to the caller to add the result to the correct
 * stat. */
static int
carrymod(enum OBJECT_ID oid) {
    int mod = 0;
    int n;

    for (n = 0; n < IVENSIZE; n++) {
        struct Object obj = Invent[n];

        if (obj.type != oid) continue;

        /* Special case 1: */
        if (oid == OBELT) {
            mod += 2 + obj.iarg*2;
            continue;
        }/* if */

        /* Special case 2: */
        if (oid == ORINGOFEXTRA) {
            mod += 5 * (obj.iarg + 1);
            continue;
        }/* if */

        /* Default behaviour. */
        mod += Types[oid].mod + obj.iarg;
    }/* for */

    return mod;
}/* carrymod */

static void
reset_stats() {
    intelligence_reset_mod();
    strength_reset_mod();
    wisdom_reset_mod();
    constitution_reset_mod();
    dexterity_reset_mod();
    charisma_reset_mod();
    defense_reset_mod();
}// reset_stats


static void
adjust_all_stat_mods(uint16_t val) {
    strength_adjust_mod(val);
    intelligence_adjust_mod(val);
    wisdom_adjust_mod(val);
    constitution_adjust_mod(val);
    dexterity_adjust_mod(val);
    charisma_adjust_mod(val);
}// adjust_all_stat_mods


// Add whatever stat mods might be caused by carrying 'obj'.
static void
recalc_carry(struct Object obj) {

    switch (obj.type) {
    case OHAMMER:
        dexterity_adjust_mod(SM_HAMMER);
        strength_adjust_mod(SM_HAMMER);
        intelligence_adjust_mod(-SM_HAMMER);
        break;

    case ODEXRING:
        dexterity_adjust_mod(obj.iarg + 1);
        break;

    case OSTRRING:
        strength_adjust_mod(obj.iarg + 1);
        break;

    case OCLEVERRING:
        intelligence_adjust_mod(obj.iarg + 1);
        break;

    case OSWORDofSLASHING:
        dexterity_adjust_mod(SM_SW_SLASHING);
        break;

    case OSLAYER:
        intelligence_adjust_mod(SM_SLAYER);
        break;

    case OPSTAFF:
        wisdom_adjust_mod(SM_STAFF);
        break;

    default:    // Why yes compiler, I *do* plan on not catching everything
        break;
    }// switch

}// recalc_carry


// Add whatever stat mods might be caused by various temporary
// effects.
static void
recalc_effects() {
    if (UU.coked) {
        adjust_all_stat_mods(SM_COKED);
    }// if

    if (UU.dexCount) {
        dexterity_adjust_mod(SM_DEX);
    }// if

    if (UU.hero) {
        adjust_all_stat_mods(SM_HEROISM);
    }// if

    if (UU.strcount) {
        strength_adjust_mod(SM_STRENGTH);
    }// if

    if (UU.giantstr) {
        strength_adjust_mod(SM_GIANTSTR);
    }// if

    if (UU.protectionTime) {
        defense_adjust_mod(SM_PROTECT);
    }// if

    if (UU.globe) {
        defense_adjust_mod(SM_INVULN);
    }// if

    if (UU.altpro) {
        defense_adjust_mod(SM_ALTPRO);
    }// if

}// recalc_effects


// recalc() function to recalculate the armor class, weapon class,
// etc. of the player.
//
// This should be called *after* regen() in the turn.
void
recalc () {
    reset_stats();

    // Defense from weapons and armor.
    defense_adjust_mod(statfor(UU.wear)+statfor(UU.shield)+carrymod(OPROTRING));

    // Check for the Eye.  This isn't strictly necessary but it
    // reinforces the principal that Invent is the single point of
    // truth for these sorts of checks.  It also minimizes the damage
    // that can be done if a bug lets the player take or drop the Eye
    // in a way that bypasses item_gain/loss_action().
    UU.hasTheEyeOfLarn = has_a(OLARNEYE);

    // Other carried
    for (int n = 0; n < IVENSIZE; n++) {
        recalc_carry(Invent[n]);
    }// for

    // Other effects (drugs, potions, spells, etc.)
    recalc_effects();
}/* recalc */


// Compute the weapon class and return it.  (This is simple enough
// that we don't need to store the actual value.)
int32_t
weaponclass() {
    return statfor(UU.wield) + carrymod(ODAMRING) + carrymod(OBELT);
}// weaponclass


/*
 *  quit()
 *
 *  subroutine to ask if the player really wants to quit
 */
void
quit () {
    int i;

    i = prompt("\n\nDo you really want to quit? (y)es, (s)ave, (n)o");

    if (i == 'y') {
        delete_save_files();
        graceful_exit("Game over, man. GAME OVER!");
        return;
    }

    if (i == 'n') {
        say(" no.\n");
        return;
    }

    save_and_quit();
}/* quit */


/* Take the object at the present location.  Return 0 on success,
 * nonzero on failure. */
void
pickup() {
    bool stat;

    stat = take(at(UU.x, UU.y)->obj, "");
    if (!stat) return;

    udelobj();
}/* pickup*/


// Perform whatever actions need to be done when picking up
// an object.
static void
item_gain_action (struct Object thing) {

    switch(thing.type) {
    case OORB:
        // We have to bump awareness right now because recalc() has
        // already been called and we want this to take effect
        // immediately.
        UU.awareness++;
        break;

    case OLARNEYE:
        // Set it now because recalc has already been called.
        UU.hasTheEyeOfLarn = true;

        if (UU.blindCount == 0) {
            say("Your sight fades for a moment...\n");
            nap(1000);
            say("Your sight returns, and everything looks crystal-clear!\n");
        }/* if */

        break;
    }/* switch*/
}/* item_gain_action */


// Perform whatever actions happen that are unique to a specific
// object or class of objects when they are received.  (Currently,
// only the Eye has one.)
static void
item_loss_action (struct Object obj) {
    if (obj.type == OLARNEYE) {

        // Set here because recalc() has already been called and also,
        // we're checking anyway.
        UU.hasTheEyeOfLarn = false;

        if (!UU.blindCount) {
            say("Your sight fades for a moment...\n");
            nap(1000);
            say("Your sight returns but everything looks dull and faded.\n");
        }// if
    }// if
}/* item_loss_action */


/* Put 'thing' in player's inventory.  If inventory is full, display
 * 'ifullMsg' unless it's NULL or empty, in which case it desplays a
 * default message.  Returns true on success, false on failure. */
bool
take (struct Object thing, const char *ifullMsg) {
    const int limit = inv_limit();

    bool foundslot = false;
    int i = 0;
    for (i = 0; i < limit; i++) {
        if (Invent[i].type==0) {
            Invent[i] = thing;
            foundslot = true;
            break;
        }/* if */
    }/* for */

    if (!foundslot) {
        if (!ifullMsg || !*ifullMsg) {
            ifullMsg = "You can't carry anything else.";
        }/* if */
        say("%s\n", ifullMsg);
        return false;
    }/* if */

    say("You pick up:\n");
    show_inv_item(i);

    item_gain_action(thing);

    return true;
}/* take */


struct Object
inventremove(int index) {
    struct Object obj = Invent[index];

    if (UU.wield == index)
        UU.wield= -1;
    if (UU.wear == index)
        UU.wear = -1;
    if (UU.shield == index)
        UU.shield= -1;

    item_loss_action(obj);

    Invent[index] = NULL_OBJ;

    return obj;
}/* inventremove*/



/*
 *  subroutine to drop an object  returns 1 if something there already else
 *
 *  k is index into iven list of object to drop
 */
int
drop_object (int k) {
    int itm, pitflag=0;
    struct Object obj;

    if (k < 0 || k > 25) { return 0; }

    itm = Invent[k].type;

    if (itm == 0) {
        say("You don't have item %c! \n",k+'a');
        return(1);
    }
    if (at(UU.x, UU.y)->obj.type == OPIT)
        pitflag=1;
    else if (at(UU.x, UU.y)->obj.type) {
        headsup();
        say("There's something here already.\n");
        return(1);
    }

    say("You drop:\n");
    show_inv_item(k); /* show what item you dropped*/

    obj = inventremove(k);
    if (!pitflag) {
        at(UU.x, UU.y)->obj = obj;
    } else {
        say("It disappears down the pit.\n");
    }/* if .. else*/

    // say dropped an item so wont ask to pick it up right away
    cancel_look();
    return(0);
}/* drop_object */



void
drop_gold (int64_t amount) {
    struct Object *o;
    int64_t dropamt = amount;

    o = &at(UU.x, UU.y)->obj;

    if (o->type == OGOLDPILE) {
        dropamt += o->iarg;
        *o = NULL_OBJ;
    }/* if */

    if (o->type && o->type != OPIT) {
        say("There's something here already!\n");
        return;
    }

    UU.gold -= amount;

    // TODO: randomly place the gold on the next level if it's not the
    // bottom.
    if (o->type == OPIT) {
        say("The gold disappears down the pit.\n");
        return;
    }/* if */

    if (dropamt > MAX_IARG) {
        UU.gold += dropamt - MAX_IARG;
        dropamt = MAX_IARG;
        say ("There isn't enough room to drop all that gold. ");
    }/* if */

    say("You drop %ld gold piece%s.\n", dropamt, (dropamt==1) ? "" :"s");

    o->type = OGOLDPILE;
    o->iarg = dropamt;

    cancel_look();

    return;
}/* drop_gold */



/*
 *  function to enchant armor player is currently wearing
 */
void
enchantarmor (enum ENCH_HOW how) {
    int which;
    int8_t *whichSlot;

    /*
     *  Bomb out if we're not wearing anything.
     */
    if (UU.wear < 0 && UU.shield < 0) {
        headsup();
        say("You feel a sense of loss.\n");
        return;
    }

    /*
     *  Choose what to enchant.
     */
    whichSlot = (rund(100) < 50) ? &UU.shield : &UU.wear;
    if (*whichSlot < 0) {
        whichSlot = (whichSlot == &UU.shield) ? &UU.wear : &UU.shield;
    }/* if */
    which = *whichSlot;

    /*
     *  Enchant it and check for destruction at >= +10.
     */
    Invent[which].iarg++;
    if (Invent[which].iarg >= 10) {
        if (how == ENCH_ALTAR) {
            Invent[which].iarg--;
            say("Your %s glows briefly.\n", objname(Invent[which]));
            return;
        }
        else if (rnd(10) <= 9) {
            ASSERT(how == ENCH_SCROLL);
            say("Your %s vibrates violently and crumbles into dust!\n",
                    objname(Invent[which]));
            Invent[which] = NULL_OBJ;
            *whichSlot = -1;
            item_loss_action(Invent[which]); /* Surely not? */
            return;
        }
    }

    say("Your %s glows for a moment.\n", objname(Invent[which]));
}/* enchantarmor */

/*
 *  function to enchant a weapon presently being wielded
 */
void
enchweapon (int how) {
    struct Object wieldedObj;
    int wieldedType;

    if (UU.wield<0) {
        headsup();
        say("You feel depressed.\n");
        return;
    }/* if */

    wieldedObj = Invent[UU.wield];
    wieldedType = wieldedObj.type;
    if (!isscroll(wieldedObj) && !ispotion(wieldedObj)) {
        Invent[UU.wield].iarg++;

        // Enchanting the relevant artifact will also boost a stat.
        if (wieldedType == OCLEVERRING)     { intelligence_adjust(1); }
        if (wieldedType == OSTRRING)        { strength_adjust(1); }
        if (wieldedType == ODEXRING)        { dexterity_adjust(1); }

        if (Invent[UU.wield].iarg >= 10 && rnd(10) <= 9) {
            if (how==ENCH_ALTAR) {
                Invent[UU.wield].iarg--;
                say("Your weapon glows a little.\n");
            }
            else {
                say("Your weapon vibrates violently and crumbles into "
                       "dust!\n");
                wieldedType=UU.wield;
                Invent[wieldedType].type = 0;
                Invent[wieldedType].iarg=0;
                item_loss_action(wieldedObj);
                UU.wield = -1;
            }
        }
        else
            say("Your weapon glows for a moment.\n");
    }
}

/* Return number of free inventory slots. */
int
inv_slots_free () {
    int count = 0;

    const int limit = inv_limit();
    for (int i = 0; i < limit; i++) {
        if (Invent[i].type == 0) {
            ++count;
        }/* if */
    }/* for */

    return count;
}/* inv_slots_free */

/*
 *  function to return 1 if a monster is next to the player else returns 0
 */
bool
nearbymonst () {
    int x, y;

    for (x = max(0, UU.x-1); x < min(MAXX, UU.x+2); x++) {
        for (y = max(0, UU.y-1); y < min(MAXY, UU.y+2); y++) {
            if (at(x, y)->mon.id) {
                return true; /* if monster nearby */
            }
        }
    }

    return false;
}

/*
 *  function to steal an item from the players pockets
 *  returns 1 if steals something else returns 0
 */
bool
stealsomething (int x, int y) {
    int index, n;

    /* Try to find a suitable item to steal. */
    for (n = 100; ; n--) {
        index=rund(IVENSIZE);

        if (n <= 0) {
            return false;
        }/* if */

        if (Invent[index].type
            && UU.wear != index
            && UU.wield != index
            && UU.shield != index) {
            break;
        }/* if */
    }/* for */

    show_inv_item(index);

    item_loss_action(Invent[index]);

    add_to_stolen (Invent[index]);

    Invent[index] = NULL_OBJ;

    return true;
}/* stealsomething */


// Test if player is carrying nothing (wielding or wearing).
bool
emptyhanded () {
    int i;

    for (i=0; i<IVENSIZE; i++) {
        if (Invent[i].type && i != UU.wield && i != UU.wear && i != UU.shield) {
            return false;
        }
    }// for

    return true;
}


/* Ensure that the base stats are at least 'min'. */
void
raise_min(uint16_t min) {
    strength_adjust_min(0, min);
    intelligence_adjust_min(0, min);
    wisdom_adjust_min(0, min);
    constitution_adjust_min(0, min);
    dexterity_adjust_min(0, min);
    charisma_adjust_min(0, min);
}/* raise_min*/

/* Add 'val' to all base stats. */
void
add_to_base_stats(int val) {
    strength_adjust(val);
    intelligence_adjust(val);
    wisdom_adjust(val);
    constitution_adjust(val);
    dexterity_adjust(val);
    charisma_adjust(val);
}/* add_to_base_stats*/


/*
 *  function to calculate the pack weight of the player
 *  returns the number of pounds the player is carrying
 */
int
packweight () {
    int n, weight;

    weight = UU.gold/1000;

    for (n = 0; n < IVENSIZE; n++) {
        uint8_t type;

        type = Invent[n].type;
        if (type == ONONE) continue;

        weight += Types[type].weight;

        /* Chests are a special case. */
        if (type == OCHEST) {
            weight += Invent[n].iarg;
        }/* if */
    }/* for */

    return weight;
}/* packweight */


bool
graduated() {
    int n;
    for (n = 0; n < EDU_LEVELS; n++) {
        if (!UU.courses[n]) return false;
    }/* for */

    return true;
}/* graduated*/



// Add 'time' (which may be negative) to each active player stat in
// UU, truncating to 1 if the result would go negative.  Used for
// stuff like scrolls of Time Warp or taking a course at U of Larn.
//
// As a special case, a true value for make_permanent (and time == 0)
// makes all active effects permanent.
void
adjust_effect_timeouts(int32_t time, bool make_permanent) {
    static int32_t *time_change[] = {
        &UU.hasteSelf, &UU.hero, &UU.altpro, &UU.protectionTime,
        &UU.dexCount, &UU.strcount, &UU.giantstr, &UU.charmcount,
        &UU.invisibility, &UU.cancellation, &UU.hasteSelf,
        &UU.aggravate, &UU.scaremonst, &UU.stealth, &UU.awareness,
        &UU.holdmonst, &UU.hastemonst, &UU.fireresistance, &UU.globe,
        &UU.spiritpro, &UU.undeadpro, &UU.halfdam, &UU.seeinvisible,
        &UU.itching, &UU.clumsiness, &UU.wtw,

        NULL};

    // Ensure the caller didn't accidently set make_permanent to true.
    ASSERT(!make_permanent || time == 0);

    for (int j = 0; time_change[j]; j++) { /* adjust time related parameters */
        // We only affect active stats
        if (*time_change[j] == 0) { continue; }

        // Permanence doesn't affect holdmonster.
        if (make_permanent && time_change[j] == &UU.holdmonst) {
            continue;
        }

        // We make stats permanent by setting them to very high values
        // which can't run out in the life of the game.  This is
        // around 10 million mobuls.  (I used to use INT32_MAX but
        // this caused the scroll of timewarp to sometimes overflow
        // the counts and lose permanent stats.)
        if (make_permanent) {
            *time_change[j] = INT32_MAX/2;
            continue;
        }// if

        // We offset by one because regen() will do the final
        // iteration.
        *time_change[j] -= (time - 1);

        // If the update took the result negative, just set it to 1
        // and let the call to regen() finish it off.
        if (*time_change[j] < 1) {
            *time_change[j] = 1;
            continue;
        }
    }/* for */

    regen();
}/* adjust_effect_timeouts*/



// Update time-based things (e.g. regenerate hitpoints and spells or
// decrease temporary effects like walk-through-walls or blindness.)
//
// This is expected to be called *before* recalc().
void
regen() {
#   define DECR(n) if(n) { --n; }

    if (UU.timestop)  {
        if(--UU.timestop <= 0) return;
    }   /* for stop time spell */

    // Advance time.  hasteSelf halves it but timestop stops it completely.
    if (UU.timestop == 0 && UU.hasteSelf % 2 == 0) {
        UU.gtime++;
    }

    /*regenerate hit points */
    if (UU.hp < UU.hpmax) {
        --UU.regencounter;

        if (UU.regencounter <= 0) {
            UU.regencounter = 22 + (2 * (int)UU.challenge) - UU.level;

            long rate = 1 + carrymod(OREGENRING) + carrymod(ORINGOFEXTRA);
            UU.hp = min(UU.hp + rate, UU.hpmax);
        }// if
    }// if

    if (UU.spells < UU.spellmax && UU.ecounter-- <= 0) { // regenerate spells
        UU.ecounter = 100 + 4 * ((int)UU.challenge - UU.level
                                 - carrymod(OENERGYRING));
        UU.spells++;
    }

    DECR(UU.hero);
    DECR(UU.coked);
    DECR(UU.altpro);

    DECR(UU.protectionTime);
    DECR(UU.dexCount);
    DECR(UU.strcount);

    if (UU.blindCount)
        if (--UU.blindCount<=0) {
            say("The blindness lifts.\n");
            headsup();
        }
    if (UU.confuse)
        if (--UU.confuse<=0) {
            say("You regain your senses.\n");
            headsup();
        }

    DECR(UU.giantstr);

    DECR(UU.charmcount);
    DECR(UU.invisibility);
    DECR(UU.cancellation);
    DECR(UU.wtw);
    DECR(UU.hasteSelf);
    DECR(UU.aggravate);
    DECR(UU.scaremonst);
    DECR(UU.stealth);
    DECR(UU.holdmonst);
    DECR(UU.hastemonst);
    DECR(UU.fireresistance);
    DECR(UU.spiritpro);
    DECR(UU.undeadpro);
    DECR(UU.enlightenment.time);
    DECR(UU.monster_detection);

    DECR(UU.globe);

    // Posessing the Orb of Awareness both preserves the existing
    // awareness count and ensures that you currently have Awareness.
    if (!has_a(OORB)) {
        DECR(UU.awareness);
    }

    if (UU.halfdam)
        if (--UU.halfdam<=0)  {
            say("You now feel better.\n");
            headsup();
        }

    if (UU.seeinvisible && !has_a(OAMULET) ) {
        if (--UU.seeinvisible <= 0) {
            say("Your vision returns to normal.\n");
            headsup();
        }
    }

    if (UU.itching) {
        if (UU.itching > 1 &&(UU.wear != -1 || UU.shield != -1)&& rnd(100)<50){
            UU.wear = UU.shield = -1;
            say("The hysteria of itching forces you to remove your armor!\n");
            headsup();
        }

        if (--UU.itching<=0) {
            say("The irritation subsides.\n");
            headsup();
        }
    }
    if (UU.clumsiness) {
        if (UU.wield != -1)
            if (UU.clumsiness>1)
                if (at(UU.x, UU.y)->obj.type==0)/* if nothing there */
                    if (rnd(100)<33) /* drop your weapon */
                        drop_object((int)UU.wield);
        if (--UU.clumsiness<=0) {
            say("You now feel less awkward.\n");
            headsup();
        }
    }
#   undef DECR
}/* regen*/


/*
 *  ****************
 *  RAISE EXPERIENCE
 *  ****************
 */


// Given an experience level, return the corresponding descriptive
// title for it.
const char *
levelDesc(int level) {
    static char mcm[] = "mighty chaos master";
    static char adg[] = "apprentice demi-god";
    static char mdg[] = "  minor demi-god   ";
    static char Mdg[] = "  major demi-god   ";
    static char mnd[] = "    minor deity    ";
    static char mjd[] = "    major deity    ";
    static char nvg[] = "  novice guardian  ";
    static char apg[] = "apprentice guardian";

    static const char *level_txt[]=
        {
            "  novice explorer  ", "apprentice explorer", " practiced explorer",/*  -3*/
            "  expert explorer  ", " novice adventurer ", "     adventurer    ",/*  -6*/
            "apprentice conjurer", "     conjurer      ", "  master conjurer  ",/*  -9*/
            "  apprentice mage  ", "       mage        ", " experienced mage  ",/* -12*/
            "    master mage    ", " apprentice warlord", "  novice warlord   ",/* -15*/
            "  expert warlord   ", "   master warlord  ", " apprentice gorgon ",/* -18*/
            "      gorgon       ", "  practiced gorgon ", "   master gorgon   ",/* -21*/
            "    demi-gorgon    ", "    chaos master   ", " great chaos master",/* -24*/
            mcm       ,   mcm       ,   mcm       ,/* -27*/
            mcm       ,   mcm       ,   mcm       ,/* -30*/
            mcm       ,   mcm       ,   mcm       ,/* -33*/
            mcm       ,   mcm       ,   mcm       ,/* -36*/
            mcm       ,   mcm       ,   mcm       ,/* -39*/
            adg       ,   adg       ,   adg       ,/* -42*/
            adg       ,   adg       ,   adg       ,/* -45*/
            adg       ,   adg       ,   adg       ,/* -48*/
            mdg       ,   mdg       ,   mdg       ,/* -51*/
            mdg       ,   mdg       ,   mdg       ,/* -54*/
            mdg       ,   mdg       ,   mdg       ,/* -57*/
            Mdg       ,   Mdg       ,   Mdg       ,/* -60*/
            Mdg       ,   Mdg       ,   Mdg       ,/* -63*/
            Mdg       ,   Mdg       ,   Mdg       ,/* -66*/
            mnd       ,   mnd       ,   mnd       ,/* -69*/
            mnd       ,   mnd       ,   mnd       ,/* -72*/
            mnd       ,   mnd       ,   mnd       ,/* -75*/
            mjd       ,   mjd       ,   mjd       ,/* -78*/
            mjd       ,   mjd       ,   mjd       ,/* -81*/
            mjd       ,   mjd       ,   mjd       ,/* -84*/
            nvg       ,   nvg       ,   nvg       ,/* -87*/
            apg       ,   apg       ,   apg       ,/* -90*/
            apg       ,   apg       ,   apg       ,/* -93*/
            "  earth guardian   ", "   air guardian    ", "   fire guardian   ",/* -96*/
            "  water guardian   ", "   time guardian   ", " ethereal guardian ",/* -99*/
            "    The Creator    "
        };

    level = max(1, min(100, level));

    ASSERT(level >= 1 && level <= 100);
    ASSERT(level - 1 < sizeof(level_txt)/sizeof(level_txt[0]));

    return level_txt[level - 1];
}// levelDesc



/*
  table of experience needed to be a certain level of player
  skill[UU.level] is the experience required to attain the next level
*/
#define MEG 1000000
static const long skill[] = {
    0, 10, 20, 40, 80, 160, 320, 640, 1280, 2560, 5120,                 /*  1-11 */
    10240, 20480, 40960, 100000, 200000, 400000, 700000, 1*MEG,         /* 12-19 */
    2*MEG,3*MEG,4*MEG,5*MEG,6*MEG,8*MEG,10*MEG,                         /* 20-26 */
    12*MEG,14*MEG,16*MEG,18*MEG,20*MEG,22*MEG,24*MEG,26*MEG,28*MEG,     /* 27-35 */
    30*MEG,32*MEG,34*MEG,36*MEG,38*MEG,40*MEG,42*MEG,44*MEG,46*MEG,     /* 36-44 */
    48*MEG,50*MEG,52*MEG,54*MEG,56*MEG,58*MEG,60*MEG,62*MEG,64*MEG,     /* 45-53 */
    66*MEG,68*MEG,70*MEG,72*MEG,74*MEG,76*MEG,78*MEG,80*MEG,82*MEG,     /* 54-62 */
    84*MEG,86*MEG,88*MEG,90*MEG,92*MEG,94*MEG,96*MEG,98*MEG,100*MEG,    /* 63-71 */
    105*MEG,110*MEG,115*MEG,120*MEG, 125*MEG, 130*MEG, 135*MEG, 140*MEG,/* 72-79 */
    145*MEG,150*MEG,155*MEG,160*MEG, 165*MEG, 170*MEG, 175*MEG, 180*MEG,/* 80-87 */
    185*MEG,190*MEG,195*MEG,200*MEG, 210*MEG, 220*MEG, 230*MEG, 240*MEG,/* 88-95 */
    260*MEG,280*MEG,300*MEG,320*MEG, 340*MEG, 370*MEG                   /* 96-101*/
};
#undef MEG

/*  subroutine to raise the player one level */
void
raiselevel () {
    if (UU.level < MAXPLEVEL) {
        raiseexperience((long)(skill[UU.level]-UU.experience));
    }// if
}// raiselevel

/*  subroutine to lower the players character level by one */
void
loselevel () {
    if (UU.level > 1) {
        loseexperience((long)(UU.experience - skill[UU.level-1] + 1));
    }// if
}/* loselevel */


/*
 *  subroutine to increase experience points
 */
void
raiseexperience (int points) {
    bool levelChanged = false;

    UU.experience += points;
    while (UU.experience >= skill[UU.level] && (UU.level < MAXPLEVEL)) {
        int hpincr = max (1, (constitution() - UU.challenge) >> 1);

        UU.level++;
        raisemhp(rnd(3) + rnd(hpincr));
        raisemspells(rund(3));

        levelChanged = true;

        if (UU.level < 7 - UU.challenge)
            raisemhp(constitution() >> 2);

        /* if we changed levels */
        switch ((int)UU.level) {
        case 94:    /* earth guardian */
            UU.wtw = INT32_MAX;
            break;
        case 95:    /* air guardian */
            UU.invisibility = INT32_MAX;
            break;
        case 96:    /* fire guardian */
            UU.fireresistance = INT32_MAX;
            break;
        case 97:    /* water guardian */
            UU.cancellation = INT32_MAX;
            break;
        case 98:    /* time guardian */
            UU.hasteSelf = INT32_MAX;
            break;
        case 99:    /* ethereal guardian */
            UU.stealth = INT32_MAX;
            UU.spiritpro = INT32_MAX;
            break;
        case 100:
            say("You are now The Creator!\n");

            set_reveal(true);

            for (int i=0; i<SPNUM; i++) {
                UU.spellknow[i] = true;
            }

            identify_all();
            break;
        }/* switch */
    }/* while */

    if (levelChanged) {
        say("Welcome to level %d!\n",(long)UU.level);
    }/* if */
}/* raiseexperience */


/*
 *  subroutine to lose experience points
 */
void
loseexperience (int x) {
    int i,tmp;

    i = UU.level;
    UU.experience -= x;
    if (UU.experience < 0) UU.experience = 0;
    while (UU.experience < skill[UU.level-1]) {
        if (--UU.level <= 1)
            UU.level=1; /*  down one level      */
        tmp = (constitution()-UU.challenge)>>1; /* lose hpoints */
        losemhp((int)rnd(tmp > 0 ? tmp : 1));   /* lose hpoints */
        if (UU.level < 7-UU.challenge)
            losemhp((int)(constitution()>>2));
        losemspells((int)rund(3));  /*  lose spells */
    }
    if (i != UU.level) {
        headsup();
        say("You went down to level %d!\n",(long)UU.level);
    }
}/* loseexperience */
