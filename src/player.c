// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include <assert.h>

#include "internal_assert.h"

#include "display.h"
#include "score_file.h"
#include "show.h"
#include "game.h"
#include "settings.h"
#include "os.h"

#include "player.h"


// The global player state object.
struct Player UU;

/* Global player inventory. */
struct Object Invent[IVENSIZE];


static void recalc(void);


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
        GS.spellknow[CMMISSILE] = true;  /* mle */
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=16;
        UU.strength = 18;   /* strength */
        UU.intelligence = 4;    /* intelligence */
        UU.wisdom = 6;  /* wisdom */
        UU.constitution = 16;   /* constitution */
        UU.dexterity = 6;   /* dexterity */
        UU.charisma = 4;    /* charisma */
        Invent[0] = random_potion();
        Invent[1] = random_potion();
        break;

    case CCWIZARD:
        GS.spellknow[CMMISSILE] = true;  /* mle */
        GS.spellknow[CCHARM] = true;  /* chm */
        UU.spellmax=UU.spells=2;
        UU.hpmax=UU.hp=8;
        UU.strength = 8;    /* strength */
        UU.intelligence = 16;   /* intelligence */
        UU.wisdom = 16; /* wisdom */
        UU.constitution = 6;    /* constitution */
        UU.dexterity = 6;   /* dexterity */
        UU.charisma = 8;    /* charisma */
        Invent[0] = obj(OPTREASURE, 0); /* potion of treasure detection */
        Invent[1] = random_scroll();
        Invent[2] = random_scroll();
        break;

    case CCKLINGON:
        GS.spellknow[CSSPEAR] = true;  /* ssp */
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=14;
        UU.strength = 14;   /* strength */
        UU.intelligence = 12;   /* intelligence */
        UU.wisdom = 4;  /* wisdom */
        UU.constitution = 12;   /* constitution */
        UU.dexterity = 8;   /* dexterity */
        UU.charisma = 3;    /* charisma */
        Invent[0] = obj(OSTUDLEATHER, 0);
        Invent[1] = random_potion();
        UU.wear = 0;
        break;

    case CCELF:
        GS.spellknow[CPROT] = true;
        UU.spells=1;
        UU.spellmax=2;
        UU.hpmax=UU.hp=8;
        UU.strength = 8;    /* strength */
        UU.intelligence = 14;   /* intelligence */
        UU.wisdom = 12; /* wisdom */
        UU.constitution = 8;    /* constitution */
        UU.dexterity = 8;   /* dexterity */
        UU.charisma = 14;   /* charisma */
        Invent[0] = obj(OLEATHER, 0);
        Invent[1] = random_scroll();
        UU.wear=0;
        break;

    case CCROGUE:
        GS.spellknow[CMMISSILE] = true;
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=12;
        UU.strength = 8;    /* strength */
        UU.intelligence = 12;   /* intelligence */
        UU.wisdom = 8;  /* wisdom */
        UU.constitution = 10;   /* constitution */
        UU.dexterity = 14;  /* dexterity */
        UU.charisma = 6;    /* charisma */

        Invent[0] = obj (OLEATHER, 0);
        Invent[1] = obj (ODAGGER, 0);
        Invent[2] = obj (OSSTEALTH, 0);  /* stealth */

        UU.wear=0;
        UU.wield=1;
        break;

    case CCGEEK:
        GS.spellknow[CPROT] = true;
        GS.spellknow[CMMISSILE] = true;
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=10;
        UU.strength = 12;   /* strength */
        UU.intelligence = 12;   /* intelligence */
        UU.wisdom = 12; /* wisdom */
        UU.constitution = 12;   /* constitution */
        UU.dexterity = 12;  /* dexterity */
        UU.charisma = 12;   /* charisma */

        Invent[0] = obj(OLEATHER, 0);
        Invent[1] = obj(ODAGGER, 0);

        UU.wear=0;
        UU.wield=1;
        break;

    case CCDWARF:
        GS.spellknow[CPROT] = true;
        UU.spellmax=UU.spells=1;
        UU.hpmax=UU.hp=12;
        UU.strength = 16;   /* strength */
        UU.intelligence = 6;    /* intelligence */
        UU.wisdom = 8;  /* wisdom */
        UU.constitution = 16;   /* constitution */
        UU.dexterity = 4;   /* dexterity */
        UU.charisma = 4;    /* charisma */

        Invent[0] = obj(OSPEAR, 0);

        UU.wield=0;
        break;

    case CCRAMBO:
        UU.spellmax=UU.spells=0;
        UU.hpmax=UU.hp=1;
        UU.strength = 3;    /* strength */
        UU.intelligence = 3;    /* intelligence */
        UU.wisdom = 3;  /* wisdom */
        UU.constitution = 3;    /* constitution */
        UU.dexterity = 3;   /* dexterity */
        UU.charisma = 3;    /* charisma */
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
init_new_player (enum CHAR_CLASS cc, enum SEX sex, int difficulty) {
    int n;

    UU.cclass = cc;
    UU.sex = sex;
    UU.challenge = difficulty;

    strncpy(UU.name, GameSettings.name, sizeof(UU.name));
    UU.name[sizeof(UU.name) - 1] = '\0';

    UU.level = 1;     /*  player starts at level one  */
    UU.regencounter = 16;
    UU.ecounter = 96;     /*start regeneration correctly*/
    UU.shield = UU.wear = UU.wield = -1;

    for (n = 0; n < IVENSIZE; n++) {
        Invent[n] = obj(ONONE, 0);
    }/* for */

    init_cc_specific(cc);

    UU.x = rnd(MAXX-2);
    UU.y = rnd(MAXY-2);
    UU.gtime = 0;    /*  time clock starts at zero   */

    // If the player previously won a game, they will owe taxes
    UU.outstanding_taxes = get_taxes_owed();

    /* Identify a potion of Cure Dianthroritis for free. */
    Types[OPCUREDIANTH].isKnown = true;

    recalc();
}/* init_new_player */


bool
has_a(enum OBJECT_ID type) {
    int n;

    for (n = 0; n < IVENSIZE; n++) {
        if (Invent[n].type == type) {
            return true;
        }
    }/* for */

    return false;
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
    int try;
    try = 2;

    while ((Map[UU.x][UU.y].obj.type || Map[UU.x][UU.y].mon.id) && (try)) {
        if (++UU.x >= MAXX-1) {
            UU.x = 1;
            if (++UU.y >= MAXY-1) {
                UU.y = 1;
                --try;
            }/* if */
        }/* if */
    }/* while */

    if (try==0)
        say("Failure in positionplayer\n");
}/* positionplayer */


/*
 *  moveplayer(dir)
 *
 * subroutine to move the player from one cell to another
 *
 * returns false if can't move in that direction or hit a monster or on an object
 * else returns true.
 *
 * If 'success' is not NULL, it is set to true if the action
 * succeeded, (i.e. the turn can continue) and false if not (i.e. the
 * move is impossible so no action took place).
 *
 * If 'success' is false, the result will also be false but the
 * reverse is not always true.  (E.g. hitting a monster doesn't move
 * the player but does consume a turn.)
 */

bool
moveplayer (DIRECTION dir, bool* success) {
    int new_x,new_y,i,j;

    if (success) *success = true;

    show1cell(UU.x,UU.y);

    if (dir == DIR_CANCEL) {
        *success = false;  /* CANCEL never succeeds 'cause it's invalid. */
        return false;
    }/* if */

    if (UU.confuse && UU.level < rnd(30) && dir != DIR_STAY) {
        dir=rund(9) + DIR_NORTH; /*if confused any dir*/
    }/* if */

    adjpoint(UU.x, UU.y, dir, &new_x, &new_y);

    if (new_x<0 || new_x>=MAXX || new_y<0 || new_y>=MAXY) {
        if (success) *success = false;
        return false;
    }

    i = Map[new_x][new_y].obj.type;
    j = Map[new_x][new_y].mon.id;

    /*  hit a wall  */
    if (i == OWALL && UU.wtw == 0) {
        bool foundit = false;

        /* If blind and the destination is unknown, reveal it but
         * spend a turn doing it. */
        if (UU.blindCount > 0 && !Map[new_x][new_y].know) {
            showcellat(new_x, new_y, 0, 0);
            foundit = true;
        }/* if */

        if (success) *success = foundit;
        return false;
    }

    if (j>0) {
        hitmonster(new_x,new_y);
        return false;
    } /* hit a monster*/

    UU.prev_x = UU.x;
    UU.prev_y = UU.y;
    UU.x = new_x;
    UU.y = new_y;

    if (i && i != OTRAPARROWIV && i != OIVTELETRAP && i != OIVDARTRAP
        && i!=OIVTRAPDOOR)
        return false;
    else
        return true;
}/* moveplayer */


/* Return the modifier field for the item at inventory slot 'slot' or
 * 0 if none is set. */
static int
statfor(int slot) {
    struct Object obj;

    if (slot < 0) return 0;
    assert(slot < IVENSIZE);

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



// recalc()    function to recalculate the armor class of the player
static void
recalc () {
    UU.ac = UU.moredefenses
        + statfor(UU.wear)
        + statfor(UU.shield)
        + carrymod(OPROTRING);

    UU.wclass = UU.moreDmg
        + statfor(UU.wield)
        + carrymod(ODAMRING)
        + carrymod(OBELT);

    UU.regen = 1
        + carrymod(OREGENRING)
        + carrymod(ORINGOFEXTRA);

    UU.energy = carrymod(OENERGYRING);
}/* recalc */


/*
 *  quit()
 *
 *  subroutine to ask if the player really wants to quit
 */
void
quit () {
    int i;

    i = prompt("\n\nDo you really want to quit? (y)es, (n)o, (s)ave");

    if (i == 'y') {
        delete_save_files();
        graceful_exit("Game over, man. GAME OVER!");
        return;
    }

    if (i == 'n' || i == 0)   {
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

    stat = take(Map[UU.x][UU.y].obj, "");
    if (!stat) return;

    udelobj();
}/* pickup*/


/* Perform whatever stat modifications need to be done when picking up
 * 'thing'. */
static bool
updatestats (struct Object thing) {
    bool changed = false;

    switch(thing.type) {
    case OPROTRING:
    case ODAMRING:
    case OBELT:
        changed = true;
        break;

    case ODEXRING:
        UU.dexterity += thing.iarg+1;
        changed = true;
        break;

    case OSTRRING:
        UU.strextra += thing.iarg+1;
        changed = true;
        break;

    case OCLEVERRING:
        UU.intelligence += thing.iarg+1;
        changed = true;
        break;

    case OHAMMER:
        UU.dexterity += 10;
        UU.strextra += 10;
        UU.intelligence_pre_hammer = UU.intelligence;
        UU.intelligence = max(UU.intelligence_pre_hammer - 10, 1);
        changed = true;
        break;

    case OORB:
        UU.orb = true;
        UU.awareness++;
        break;

    case OORBOFDRAGON:
        UU.slaying++;
        break;

    case OSPIRITSCARAB:
        UU.negatespirit++;
        break;

    case OCUBE_of_UNDEAD:
        UU.cube_of_undead++;
        break;

    case ONOTHEFT:
        UU.notheft++;
        break;

    case OSWORDofSLASHING:
        UU.dexterity +=5;
        changed = true;
        break;

    case OSLAYER:
        UU.intelligence+=10;
        break;

    case OPSTAFF:
        UU.wisdom+=10;
        break;

    case  OLARNEYE:
        UU.eyeOfLarn = 1;
        break;
    }/* switch*/

    return changed;
}/* updatestats */



/* Put 'thing' in player's inventory.  If inventory is full, display
 * 'ifullMsg' unless it's NULL or empty, in which case it desplays a
 * default message.  Returns true on success, false on failure. */
int
take (struct Object thing, const char *ifullMsg) {
    int limit, i;
    bool foundslot;

    if ((limit = 15+(UU.level>>1)) > IVENSIZE) {
        limit = IVENSIZE;
    }/* if */

    foundslot = false;
    for (i=0; i<limit; i++) {
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

    updatestats(thing);

    say("You pick up:\n");
    show_inv_item(i);

    if (UU.blindCount == 0 && thing.type == OLARNEYE) {
        say("Your sight fades for a moment...\n");
        nap(2000);
        update_display(true);
        say("Your sight returns, and everything looks crystal-clear!\n");
    }/* if */

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

    adjustcvalues(obj.type, obj.iarg);

    if (obj.type == OLANCE)  {
        recalc();
    }/* if */

    Invent[index] = NullObj;

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

    if ((k<0) || (k>25))
        return(0);

    itm = Invent[k].type;

    if (itm==0) {
        say("You don't have item %c! \n",k+'a');
        return(1);
    }
    if (Map[UU.x][UU.y].obj.type == OPIT)
        pitflag=1;
    else if (Map[UU.x][UU.y].obj.type) {
        headsup();
        say("There's something here already.\n");
        return(1);
    }

    say("You drop:\n");
    show_inv_item(k); /* show what item you dropped*/

    obj = inventremove(k);
    if (!pitflag) {
        Map[UU.x][UU.y].obj = obj;
    } else {
        say("It disappears down the pit.\n");
    }/* if .. else*/

    Map[UU.x][UU.y].know = 0;

/* say dropped an item so wont ask to pick it up right away */
    cancelLook();
    return(0);
}/* drop_object */



void
drop_gold (unsigned long amount) {
    struct Object *o;
    unsigned dropamt = amount;

    o = &Map[UU.x][UU.y].obj;

    if (o->type == OGOLDPILE) {
        dropamt += o->iarg;
        *o = NullObj;
    }/* if */

    if (o->type && o->type != OPIT) {
        say("There's something here already!\n");
        return;
    }

    UU.gold -= amount;

    if (o->type == OPIT) {
        say("The gold disappears down the pit.\n");
        return;
    }/* if */

    if (dropamt > MAX_IARG) {
        UU.gold += dropamt - MAX_IARG;
        dropamt = MAX_IARG;
        say ("There isn't enough room to drop all that gold. ");
    }/* if */

    say("You drop %ld gold piece%s.", dropamt, (dropamt==1) ? "" :"s");

    o->type = OGOLDPILE;
    o->iarg = dropamt;

    update_stats();

    Map[UU.x][UU.y].know=0;
    cancelLook();

    return;
}/* drop_gold */



/*
 *  function to enchant armor player is currently wearing
 */
void
enchantarmor (enum ENCH_HOW how) {
    int which;
    long *whichSlot;

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
            Invent[which] = NullObj;
            *whichSlot = -1;
            adjustcvalues(Invent[which].type, Invent[which].iarg); /* Surely not? */
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
        if (wieldedType==OCLEVERRING)
            UU.intelligence++;
        else if (wieldedType==OSTRRING)
            UU.strextra++;
        else if (wieldedType==ODEXRING)
            UU.dexterity++;
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
                adjustcvalues(wieldedObj.type, wieldedObj.iarg);
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
    int i, limit, count;

    // FIX: violates DRY principal
    if ((limit = 15+(UU.level>>1)) > IVENSIZE) {
        limit = IVENSIZE;
    }/* if */

    for (i = 0, count = 0; i < limit; i++) {
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
            if (Map[x][y].mon.id) {
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

    adjustcvalues(Invent[index].type, Invent[index].iarg);

    add_to_stolen (Invent[index], Lev);

    Invent[index] = NullObj;

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
    createitem(UU.x, UU.y, i, rnd(j)+(j/10));
}/* creategem */

/*
 *  function to change character levels as needed when dropping an object
 *  that affects these characteristics
 */
void
adjustcvalues (int itm, int arg) {
    if (ispotion(obj(itm, arg))) {
        return;
    }/* if */

    switch(itm) {

    case ODEXRING:
        UU.dexterity -= arg+1;
        break;

    case OSTRRING:
        UU.strextra  -= arg+1;
        break;

    case OCLEVERRING:
        UU.intelligence -= arg+1;
        break;

    case OHAMMER:
        UU.dexterity -= 10;
        UU.strextra -= 10;
        UU.intelligence = UU.intelligence_pre_hammer;
        break;

    case OORB:
        UU.orb = false;
        UU.awareness--;
        break;

    case OSWORDofSLASHING:
        UU.dexterity -= 5;
        break;

    case OSLAYER:
        UU.intelligence-=10;
        break;

    case OPSTAFF:
        UU.wisdom-=10;
        break;

    case OORBOFDRAGON:
        --UU.slaying;
        return;

    case OSPIRITSCARAB:
        --UU.negatespirit;
        return;

    case OCUBE_of_UNDEAD:
        --UU.cube_of_undead;
        return;

    case ONOTHEFT:
        --UU.notheft;
        return;

    case OLARNEYE:
        UU.eyeOfLarn = 0;
        if (UU.blindCount == 0) {
            say("Your sight fades for a moment...\n");
            nap(2000);
            update_display(true);
            say("Your sight returns but everything looks dull and faded.\n");
        }
        return;
    }

    raise_min(3);
}/* adjustcvalues */


/* Ensure that the base stats are at least 'min'. */
void
raise_min(int min) {
    UU.strength     = (UU.strength < min)     ? min : UU.strength;
    UU.intelligence = (UU.intelligence < min) ? min : UU.intelligence;
    UU.wisdom       = (UU.wisdom < min)       ? min : UU.wisdom;
    UU.constitution = (UU.constitution < min) ? min : UU.constitution;
    UU.dexterity    = (UU.dexterity < min)    ? min : UU.dexterity;
    UU.charisma     = (UU.charisma < min)     ? min : UU.charisma;
}/* raise_min*/

/* Add 'val' to all base stats. */
void
add_to_base_stats(int val) {
    UU.strength     += val;
    UU.intelligence += val;
    UU.wisdom       += val;
    UU.constitution += val;
    UU.dexterity    += val;
    UU.charisma     += val;
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
graduated(struct Player *p) {
    int n;
    for (n = 0; n < EDU_LEVELS; n++) {
        if (!p->courses[n]) return false;
    }/* for */

    return true;
}/* graduated*/



// Update time-based fields in UU by a larger amount.  Used for stuff
// like scrolls of Time Warp or taking a course at U of Larn.
void
adjusttime(long tim) {
    static long *time_change[] = {
        &UU.hasteSelf, &UU.hero, &UU.altpro, &UU.protectionTime,
        &UU.dexCount, &UU.strcount, &UU.giantstr, &UU.charmcount,
        &UU.invisibility, &UU.cancellation, &UU.hasteSelf,
        &UU.aggravate, &UU.scaremonst, &UU.stealth, &UU.awareness,
        &UU.holdmonst, &UU.hastemonst, &UU.fireresistance, &UU.globe,
        &UU.spiritpro, &UU.undeadpro, &UU.halfdam, &UU.seeinvisible,
        &UU.itching, &UU.clumsiness, &UU.wtw,

        NULL};
    int j;

    for (j=0; time_change[j]; j++) { /* adjust time related parameters */
        if (*(time_change[j])) {
            if (*(time_change[j]) < tim+1) {
                *(time_change[j]) = 1;
            } else {
                *(time_change[j]) -= tim;
            }/* if .. else*/
        }/* if */
    }/* for */

    regen();
}/* adjusttime*/



// Update time-based things (e.g. regenerate hitpoints and spells or
// decrease temporary effects like walk-through-walls or blindness.)
void
regen() {
#   define DECR(n) if(n) --n

    if (UU.timestop)  {
        if(--UU.timestop<=0)
        return;
    }   /* for stop time spell */

    if (UU.strength<3) {
        UU.strength=3;
    }
    if (UU.hasteSelf==0)        // XXXX probably wrong
        UU.gtime++;

    if (UU.hp != UU.hpmax)
        if (UU.regencounter-- <= 0) /*regenerate hit points */
        {
            UU.regencounter = 22 + (UU.challenge<<1) - UU.level;
            if ((UU.hp += UU.regen) > UU.hpmax)
                UU.hp = UU.hpmax;
        }

    if (UU.spells < UU.spellmax)        /*regenerate spells */
        if (UU.ecounter-- <= 0) {
            UU.ecounter = 100+4*(UU.challenge-UU.level-UU.energy);
            UU.spells++;
        }

    if (UU.hero)
        if (--UU.hero <= 0) {
            add_to_base_stats(-10);
        }
    if (UU.coked)
        if (--UU.coked<=0) {
            add_to_base_stats(-34);
        }
    if (UU.altpro)
        if (--UU.altpro<=0) {
            UU.moredefenses-=3;
        }
    if (UU.protectionTime)
        if (--UU.protectionTime<=0) {
            UU.moredefenses-=2;
        }
    if (UU.dexCount)
        if (--UU.dexCount<=0)   {
            if ( (UU.dexterity-=3) < 3 )
                UU.dexterity = 3;
        }
    if (UU.strcount)
        if (--UU.strcount<=0)   {
            UU.strextra-=3;
        }
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
    if (UU.giantstr)
        if (--UU.giantstr<=0) {
            UU.strextra -= 20;
        }

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

    if (UU.globe)
        if (--UU.globe<=0) {
            UU.moredefenses-=10;
        }

    if (UU.awareness)
        if(!UU.orb)
            --UU.awareness;
    if (UU.halfdam)
        if (--UU.halfdam<=0)  {
            say("You now feel better.\n");
            headsup();
        }
    if (UU.seeinvisible)  {
        int i;
        for (i=0;i<IVENSIZE;i++)
            if (Invent[i].type==OAMULET) {
                i=999;
                break;
            }
        if (i!=999)
            if (--UU.seeinvisible<=0) {
                MonType[INVISIBLESTALKER].mapchar=Types[0].symbol;
                say("Your vision returns to normal.\n");
                headsup();
            }
    }

    if (UU.itching) {
        if (UU.itching > 1 && (UU.wear!= -1 || UU.shield!= -1) && rnd(100)<50){
            UU.wear=UU.shield= -1;
            say("The hysteria of itching forces you to remove \n"
                   "your armor!");
            headsup();
            recalc();
        }

        if (--UU.itching<=0) {
            say("The irritation subsides.\n");
            headsup();
        }
    }
    if (UU.clumsiness) {
        if (UU.wield != -1)
            if (UU.clumsiness>1)
                if (Map[UU.x][UU.y].obj.type==0)/* if nothing there */
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
raiseexperience (long points) {
    bool levelChanged = false;

    UU.experience += points;
    while (UU.experience >= skill[UU.level] && (UU.level < MAXPLEVEL)) {
        int hpincr = max (1, (UU.constitution - UU.challenge) >> 1);

        UU.level++;
        raisemhp(rnd(3) + rnd(hpincr));
        raisemspells(rund(3));

        levelChanged = true;

        if (UU.level < 7 - UU.challenge)
            raisemhp((int)(UU.constitution>>2));

        /* if we changed levels */
        switch ((int)UU.level) {
        case 94:    /* earth guardian */
            UU.wtw = 99999L;
            break;
        case 95:    /* air guardian */
            UU.invisibility = 99999L;
            break;
        case 96:    /* fire guardian */
            UU.fireresistance = 99999L;
            break;
        case 97:    /* water guardian */
            UU.cancellation = 99999L;
            break;
        case 98:    /* time guardian */
            UU.hasteSelf = 99999L;
            break;
        case 99:    /* ethereal guardian */
            UU.stealth = 99999L;
            UU.spiritpro = 99999L;
            break;
        case 100:
            say("You are now The Creator!\n");
            {
                int i,j;

                for (i=0; i<MAXY; i++)
                    for (j=0; j<MAXX; j++)
                        Map[j][i].know=1;
                for (i=0; i<SPNUM; i++)
                    GS.spellknow[i] = true;
                for (i=0; i < OBJ_COUNT; i++)
                    Types[i].isKnown = true;
            }
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
loseexperience (long x) {
    int i,tmp;

    i = UU.level;
    UU.experience -= x;
    if (UU.experience < 0) UU.experience = 0;
    while (UU.experience < skill[UU.level-1]) {
        if (--UU.level <= 1)
            UU.level=1; /*  down one level      */
        tmp = (UU.constitution-UU.challenge)>>1; /* lose hpoints */
        losemhp((int)rnd(tmp > 0 ? tmp : 1));   /* lose hpoints */
        if (UU.level < 7-UU.challenge)
            losemhp((int)(UU.constitution>>2));
        losemspells((int)rund(3));  /*  lose spells */
    }
    if (i != UU.level) {
        headsup();
        say("You went down to level %d!\n",(long)UU.level);
    }
}/* loseexperience */
