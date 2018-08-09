// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "cast.h"

#include "internal_assert.h"

#include "display.h"
#include "sphere.h"
#include "movem.h"
#include "game.h"

#include <limits.h>


/* Create the big list of spells. */
const struct SpellInfo Spells[] = {

#define SPELL(id, code, name, ldesc) {code,name,ldesc},
#include "spell_list.h"
#undef SPELL

};

static void spelldamage(enum SPELL spell);
static int nospell(enum SPELL spell, int monst);
static void direct(enum SPELL spell, int dam, char *str, int arg);
static void dirpoly(void);
static void omnidirect(enum SPELL spell, int dam, char *str);
static void tdirect(void);
static void makewall(void);
static void vaporizerock(void);
static DIRECTION dirsub(int *x, int *y);
static void alter_reality(void);
static int isconfuse(void);
static void loseint(void);




/* Display a list of spells and let the player pick one.  'mode'
 * determines why: casting, learning (from a genie) or displaying.
 * Only known spells are displayed unless mode is 'learn' in which
 * case only unknown spells are shown.  'mode' also determines the
 * heading.  Result is the index of the spell selected or -1 if
 * none. */
int
pickspell (enum PICKMODE mode) {
    struct PickList *picker;
    int n, result = -1;
    const char *heading;
    bool known = (mode != PM_LEARN);

    ASSERT (mode == PM_CAST || mode == PM_LEARN);

    if (mode != PM_LEARN && UU.spells <= 0) {
        say("You don't have any spells!\n");
        return -1;
    }/* if */

    picker = pl_malloc();

    for (n = 0; n < SPNUM; n++) {
        char letter;
        char desc[100];

        if (known != GS.spellknow[n]) continue;

        snprintf (desc, sizeof(desc), "%-20s %s", Spells[n].name,
                  Spells[n].desc);
        letter = (n >= 26) ? 'A' + (n - 26) : 'a' + n;
        pl_add (picker, n, letter, desc);
    }/* for */

    heading = mode == PM_CAST  ? "Cast which spell?"  : 
              mode == PM_LEARN ? "Learn which spell?" :
                                 "-=*=-";
                 
    pick_item (picker, heading, &result);

    pl_free(picker);

    return result;
}/* pickspell */



/*
 * Subroutine called by play_turn to cast a spell for the user 
 */
void
cast() {
    int spell;

    spell = pickspell(PM_CAST);
    if (spell < 0) {
        return;
    }/* if */

    --UU.spells;

    spelldamage(spell);
}/* cast*/




/*
 * Function to perform spell functions cast by the player.
 *
 * Takes  the spell number, returns no value.
 */
void
spelldamage(enum SPELL spell) {

    if (spell >= SPNUM)
        return;     /* no such spell */

    if (UU.timestop) {
        say("It didn't seem to work.\n");
        return;
    }           /* not if time stopped */

    if ((rnd(23) == 7) || (rnd(18) > UU.intelligence)) {
        say("It didn't work!\n");
        return;
    }/* if */

    if (UU.level * 3 + 2 < spell) {
        say("Nothing happens.  You seem inexperienced.\n");
        return;
    }/* if */

    switch (spell) {
        /* ----- LEVEL 1 SPELLS ----- */

    case CPROT:
        if (UU.protectionTime == 0)
            UU.moredefenses += 2;   /* protection field +2 */
        UU.protectionTime += 250;
        return;

    case CMMISSILE: {    /* magic missile */
        int i = rnd(((UU.level + 1) << 1)) + UU.level + 3;
        godirect(spell, i, (UU.level >= 2) ?
                 "Your missiles hit the %s." :
                 "Your missile hit the %s.", 25, '+');
        return;
    }

    case CDEX:     /* dexterity     */
        if (UU.dexCount == 0)
            UU.dexterity += 3;
        UU.dexCount += 400;
        return;

    case CSLEEP: {    /* sleep     */
        int i;
        char *s;

        i = rnd(3) + 1;
        s = "While the %s slept, you smashed it %d times.";
        direct(spell, fullhit(i), s, i);
        return;
    }

    case CCHARM:     /* charm monster */
        UU.charmcount += UU.charisma * 2;
        return;

    case CSSPEAR:     /* sonic spear */
        godirect(spell, rnd(10) + 15 + UU.level, 
                 "The sound damages the %s.", 70, '@');
        return;

        /* ----- LEVEL 2 SPELLS ----- */
    case CWEB: {
        int i = rnd(3) + 2;
        char *s = "While the %s is entangled, you hit it %d times.";
        direct(spell, fullhit(i), s, i);
        return;
    }

    case CSTR:
        if (UU.strcount == 0)
            UU.strextra += 3;   /* strength */
        UU.strcount += 150 + rnd(100);
        return;

    case CENLIGHTEN: 
        enlighten(15, 6);
        return;

    case CHEALING:     /* healing */
        raisehp(20 + (UU.level << 1));
        return;

    case CCBLIND:        /* cure blindness    */
        UU.blindCount = 0;
        return;

    case CCREATEMON:
        createmonster(makemonst(getlevel() + 1) + 8);
        return;

    case CPHANTASM:
        if (rnd(11) + 7 <= UU.wisdom)
            direct(spell, rnd(20) + 20 + UU.level, "The %s believed!", 0);
        else
            say("It didn't believe the illusions!");
        return;


    case CINV: {       /* if he has the amulet of invisibility then
                        * add more time */
        int i, j;

        for (j = i = 0; i < IVENSIZE; i++)
            if (Invent[i].type == OAMULET)
                j += 1 + Invent[i].iarg;
        UU.invisibility += (j << 7) + 12;
        return;
    }

        /* ----- LEVEL 3 SPELLS ----- */
    case CFIREBALL:
        godirect(spell, rnd(25 + UU.level) + 25 + UU.level,
                 "The fireball hits the %s.", 40, '*');
        return;     /* fireball */

    case CCOLD:        /* cold */
        godirect(spell, rnd(25) + 20 + UU.level,
                 "The cone of cold strikes the %s.", 60, 'O');
        return;

    case CPOLY:        /* polymorph */
        dirpoly();
        return;

    case CCANCEL:      /* cancellation  */
        UU.cancellation += 5 + UU.level;
        return;

    case CHASTE:        /* haste self    */
        UU.hasteSelf += 7 + UU.level;
        return;

    case CCLOUD:        /* cloud kill */
        omnidirect(spell, 30 + rnd(10), "The %s gasps for air!\n");
        return;

    case CVPROCK:        /* vaporize rock */
        vaporizerock();
        return;

        /* ----- LEVEL 4 SPELLS ----- */
    case CDRY:        /* dehydration */
        direct(spell, 100 + UU.level, "The %s shrivels up.", 0);
        return;

    case CLIGHTNING:        /* lightning */
        godirect(spell, rnd(25) + 20 + (UU.level << 1),
                 "A lightning bolt hits the %s.", 1, '~');
        return;

    case CDRAIN: {       /* drain life */
        int i;

        i = min(UU.hp - 1, UU.hpmax / 2);
        direct(spell, i + i, "", 0);
        UU.hp -= i;
        return;
    }

    case CINVULN:
        if (UU.globe == 0)
            UU.moredefenses += 10;
        UU.globe += 200;
        loseint();  /* globe of invulnerability */
        return;

    case CFLOOD:        /* flood */
        omnidirect(spell, 32 + UU.level,
                   "The %s struggles for air in your flood!\n");
        return;

    case CFINGER:        /* finger of death */
        if (rnd(151) == 63) {
            headsup();
            say("Your heart stopped!\n\n");
            nap(4000);
            game_over_probably(DDFINGER);
            return;
        }
        if (UU.wisdom > rnd(10) + 10)
            direct(spell, 2000,
                   "The %s's heart stopped.", 0);
        else
            say("It didn't work.");
        return;

        /* ----- LEVEL 5 SPELLS ----- */

    case CSCAREMON: {        /* scare monster */
        UU.scaremonst += rnd(10) + UU.level;

        /* if have HANDofFEAR make last longer */
        if (has_a(OHANDofFEAR)) {
            UU.scaremonst *= 3;
        }/* if */
        return;
    }

    case CHOLDMON:        /* hold monster */
        UU.holdmonst += rnd(10) + UU.level;
        return;

    case CTIMESTOP:
        UU.timestop += rnd(20) + (UU.level << 1);
        return;     /* time stop */

    case CTELEPORT:        /* teleport */
        tdirect();
        return;

    case CMAGICFIRE:        /* magic fire */
        omnidirect(spell, 35 + rnd(10) + UU.level,
                   "The %s cringes from the flame.\n");
        return;

        /* ----- LEVEL 6 SPELLS ----- */
    case CMKWALL:        /* make wall */
        makewall();
        return;

    case CSPHERE: {       /* sphere of annihilation */
        int xl, yl, i;

        i = dirsub(&xl, &yl);   /* get direction of sphere */
        if (i == DIR_CANCEL) return;

        if (rnd(23) == 5) {
            headsup();
            say("You have been enveloped by the zone of nothingness!\n\n");
            nap(4000);
            game_over_probably(DDSELFANNIH);
            return;
        }
        loseint();
        newsphere(xl, yl, i, rnd(20) + 11); /* make a sphere */
        return;
    }

    case CGENOCIDE:        /* genocide */
        genmonst();
        return;

    case CSUMMON: {       /* summon demon */
        int i;

        if (rnd(100) > 30) {
            direct(spell, 150, "The demon strikes at the %s.", 0);
            return;
        }
        if (rnd(100) > 15) {
            say("Nothing seems to have happened.");
            return;
        }
        say("The demon clawed you and vanished!");
        headsup();
        i = rnd(40) + 30;
        losehp(i, DDDEMON);  /* must say killed by a demon */
        return;
    }

    case CWALLWALK:        /* walk through walls */
        UU.wtw += rnd(10) + 5;
        return;

    case CALTER_REALITY:        /* alter reality */
        alter_reality();
        return;

    case CPERMANENCE:        /* permanence */
        adjusttime(LONG_MIN);
        GS.spellknow[CPERMANENCE] = false;   /* forget */
        loseint();
        return;

    default:
        say("spell %d not available!", (long) spell);
        headsup();
        return;
    }
}/* spelldamage*/

/*
 * Subtract 1 from your int (intelligence) if > 3 
 */
static void
loseint() {
    if (--UU.intelligence < 3)
        UU.intelligence = 3;
}/* loseint*/


/*
 * isconfuse()      Routine to check to see if player is confused 
 *
 * This routine prints out a message saying "You can't aim your magic!" returns
 * 0 if not confused, non-zero (time remaining confused) if confused 
 */
static int
isconfuse() {
    if (UU.confuse) {
        say(" You can't aim your magic!");
        headsup();
    }
    return (UU.confuse);
}

/*
 * Subroutine to return 1 if the spell can't affect the monster otherwise
 * returns 0. Enter with the spell number in x, and the monster number in
 * monst. 
 */
static int
nospell(enum SPELL spell, int monst) {
    static const char *spellEffects[NUM_MONSTERS+1][SPNUM];
    static bool spellEffectsInitialized = false;
    const char *msg;
    
    /* bad spell or monst */
    ASSERT(spell < SPNUM && monst <= NUM_MONSTERS && monst > 0 && spell >= 0);

    /* The first time nospell is called, we initialize spellEffects
     * from the data in spell_immunities.h. */
    if (!spellEffectsInitialized) {
        struct {
            const char *msg;
            struct {
                enum MONSTER_ID mon;
                enum SPELL spell;
            } imm[120];
        } mondesc[] = {
#           include "spell_immunities.h"
        };
        int n, i;

        for (n = 0; mondesc[n].msg; n++) {
            for (i = 0; mondesc[n].imm[i].mon; i++) {
                int mon   = mondesc[n].imm[i].mon;
                int spell = mondesc[n].imm[i].spell;

                ASSERT (mon <= NUM_MONSTERS && spell < SPNUM);
                spellEffects[mon][spell] = mondesc[n].msg;
            }/* for */
        }/* for */
        
        spellEffectsInitialized = true;
    }/* if */

    msg = spellEffects[monst][spell];
    if (!msg) {
        return (0);
    }/* if */

    say(msg, monname(monst));
    say("\n");
    return (1);
}/* nospell*/


/*
 * Function to perform missile attacks.
 *
 * Function to hit in a direction from a missile weapon and have it keep on
 * going in that direction until its power is exhausted. With the spell
 * number in spnum, the power of the weapon in hp, say() format string in
 * str, the # of milliseconds to delay between locations in delay, and the
 * character to represent the weapon in cshow.
 *
 * Returns no value. 
 */
void
godirect(enum SPELL spnum, int dam, char *str, int delay, char cshow) {
    uint8_t *it;
    int x, y, m;
    int dx, dy;

    if (isconfuse()) {
        return;
    }

    // Set dx and dy to increment
    if (dirsub(&dx, &dy) == DIR_CANCEL) return;
    dx -= UU.x;
    dy -= UU.y;

    
    x = UU.x;
    y = UU.y;

    while (dam > 0) {
        x += dx;
        y += dy;
        if ((x > MAXX - 1) || (y > MAXY - 1) || (x < 0) || (y < 0)) {
            dam = 0;
            break;  /* out of bounds */
        }
        /* if energy hits player */
        if ((x == UU.x) && (y == UU.y)) {
            say("You are hit by your own magic!\n");
            headsup();
            losehp(dam, DDMAGICOOPS);
            return;
        }

        /* if not blind show effect */
        if (UU.blindCount == 0) {
            mapdraw(x, y, cshow, MF_EFFECT);
            sync_ui(false);
            nap(delay);
            show1cell(x, y);
        }/* if */

        /* is there a monster there? */
        m = Map[x][y].mon.id;
        if (m != 0) {
            /* cannot cast a missile spell at the demon king!! */
            if (m == DEMONKING || (m >= DEMONLORD1 && rnd(100) < 10)){
                dx *= -1;
                dy *= -1;
                say("The %s returns your puny missile!\n", monname(m));
            } else {
                if (nospell(spnum, m)) {
                    lasthit(x,y);
                    return;
                }
                say(str, monname_at(x, y));
                say("\n");
                dam -= hitm(x, y, dam);
                show1cell(x, y);
                nap(1000);
                x -= dx;
                y -= dy;
            }
        } else {
            it = &Map[x][y].obj.type;
            switch (*it) {
            case OWALL:
                say(str, "wall");
                
                if (dam >= 50 + UU.challenge) /*enough damage?*/
                    /* can't break wall below V2 */
                    if (getlevel() < VBOTTOM-2)
                        if ((x < MAXX - 1) && (y < MAXY - 1) 
                            && (x) && (y)) {
                            say(" The wall crumbles.");
                            *it = 0;
                            Map[x][y].know = 0;
                            show1cell(x, y);
                        }
                say("\n");
                dam = 0;
                break;

            case OCLOSEDDOOR:
                say(str, "door");
                if (dam >= 40) {
                    say("The door is blasted apart.");
                    *it = 0;
                    Map[x][y].know = 0;
                    show1cell(x, y);
                }
                say("\n");
                dam = 0;
                break;

            case OSTATUE:
                say(str, "statue");
                if (dam > 44) {
                    if (UU.challenge > 3 && rnd(60) < 30) {
                        dam = 0;
                        break;
                    }/* if */
                    say("The statue crumbles.");
                    *it = OBOOK;
                    Map[x][y].obj.iarg = getlevel();
                    Map[x][y].know = false;
                    show1cell(x, y);
                }/* if */
                say("\n");
                dam = 0;
                break;

            case OTHRONE:
                say(str, "throne");
                if (dam > 33) {
                    Map[x][y].mon = mk_mon(GNOMEKING);
                    *it = OTHRONE2;
                    Map[x][y].know = false;
                    show1cell(x, y);
                }
                say("\n");
                dam = 0;
                break;

            case OMIRROR:
                dx *= -1;
                dy *= -1;
                break;

                /* Most buildings just absorb the attack harmlessly */
            case OSCHOOL:
            case OTRADEPOST:
            case ODNDSTORE:
            case OPAD:
            case OHOME:
                dam = 0;
                break;

                /* Bank of Larn: We're a Monopoly, and it Shows. */
            case OBANK:
            case OBANK2:
                say ("The bank returns your missile with interest!\n"
                         "(And charges you a fee for the service.)\n");
                UU.bankaccount = max(0, UU.bankaccount - 2000);

                dx *= -1;
                dy *= -1;
                dam *= 2;
                break;

                /* What are you, some kind of flag-fringe tax nut? */
            case OLRS:        
                if (dam < 60 + UU.challenge) {
                    say("The LRS absorbs your effort.\n");
                } else {
                    say ("You break some windows at the LRS office.\n"
                         "The LRS adds the repair costs to your tax bill.\n");
                    UU.outstanding_taxes += 400 + rund(1000);
                };
                dam = 0;
                break;
            }/* switch */
        }/* if .. else*/
        dam -= 3 + (int) (UU.challenge >> 1);
    }
}/* godirect*/



/*
 * Routine to teleport away a monster.
 *
 * Routine to ask for a direction to a spell and then teleport a monster away.
 */
static void
tdirect(){
    int x, y, m;

    if (isconfuse()) return;

    if (dirsub(&x, &y) == DIR_CANCEL) return;
    if ((m = Map[x][y].mon.id) == 0) {
        say("There wasn't anything there!");
        return;
    }
    if (nospell(CTELEPORT, m)) {
        lasthit(x, y);
        return;
    }
    teleportmonst(x,y);
}/* tdirect*/



/* Create a wall in the requested direction. */
static void
makewall() {
    int x, y;

    if (isconfuse()) return;
    if (dirsub(&x, &y) == DIR_CANCEL) return;

    if ((y >= 0) && (y <= MAXY - 1) && (x >= 0) && (x <= MAXX - 1)) {/* within bounds? */
        if (Map[x][y].obj.type != OWALL) {  /* can't make anything on
                                             * walls */
            if (Map[x][y].obj.type == 0) {  /* is it free of items? */
                if (Map[x][y].mon.id == 0) { /* is it free of
                                             * monsters? */
                    if ((getlevel() != 1) || (x != 33) || (y != MAXY - 1)) {    // <- redundant
                        Map[x][y].obj = obj(OWALL, 0);
                        Map[x][y].know = 1;
                        show1cell(x, y);
                    } else
                        say("you can't make a wall there!\n");
                } else
                    say("there's a monster there!\n");
            } else
                say("there's something there already!\n");
        } else {
            say("there's a wall there already!\n");
        }/* if .. else*/
    }/* if */
}/* makewall*/


/* Perform the 'vaporize rock' spell */
static void
vaporizerock() {
    int xh, yh;
    int i, j;

    xh = min(UU.x + 1, MAXX - 2);
    yh = min(UU.y + 1, MAXY - 2);
    for (i = max(UU.x - 1, 1); i <= xh; i++) {
        for (j = max(UU.y - 1, 1); j <= yh; j++) {
            struct MapSquare *pt = &Map[i][j];

            switch (pt->obj.type) {
            case OWALL:
                /* can't vpr below V2 */
                if (getlevel() < VBOTTOM-2) {
                    pt->obj = NullObj;
                    pt->know = false;
                }/* if */
                break;

            case OSTATUE:
                if (UU.challenge > 3 && rnd(60) < 30) {
                    break;
                }/* if */
                pt->obj = obj (OBOOK, getlevel());
                pt->know = false;
                break;

            case OTHRONE:
                pt->mon = mk_mon(GNOMEKING);
                pt->know = false;
                pt->obj = obj(OTHRONE2, 0);
                break;

            case OALTAR:
                pt->mon = mk_mon(DEMONPRINCE);
                pt->know = false;
                createmonster(DEMONPRINCE);
                createmonster(DEMONPRINCE);
                createmonster(DEMONPRINCE);
                createmonster(DEMONPRINCE);
                break;
            }/* switch */

                
            switch (pt->mon.id) {
                /* Xorn takes damage from vpr */
            case XORN:
                hitm(i, j, 200);
                break;

            case TROLL:
                hitm(i, j, 200);
                break;
            }/* switch */
        }/* for */
    }/* for */
}/* vaporizerock*/


// Perform the 'alter reality' spell (i.e. recreate the current map
// level while keeping the objects and creatures.
static void
alter_reality() {
    remake_map_keeping_contents();

    loseint();
    update_display(true);
    
    GS.spellknow[CALTER_REALITY] = false;

    positionplayer();
}/* alter_reality*/ 


/*
 * Routine to damage all monsters 1 square from player 
 *
 * Enter with the spell number in sp, the damage done to wach square
 * in dam, and the format string to identify the spell in str. Returns
 * no value.
 */
static void
omnidirect(enum SPELL spell, int dam, char *str) {
    int x, y, m;

    for (x = UU.x - 1; x < UU.x + 2; x++)
        for (y = UU.y - 1; y < UU.y + 2; y++) {
            if ((m = Map[x][y].mon.id) != 0) {
                if (nospell(spell, m) == 0) {
                    say(str, monname_at(x, y));
                    hitm(x, y, dam);
                    nap(800);
                } else {
                    lasthit(x, y);
                }/* if .. else*/
            }/* if */
        }/* for */
}/* omnidirect*/


// Prompt the user for a direction, then set *x and *y to the point
// relative the the player in that direction.
static DIRECTION
dirsub(int *x, int *y) {
    DIRECTION dir;

    dir = promptdir(true);
    adjpoint(UU.x, UU.y, dir, x, y);
    VXY(*x, *y);
    
    return dir;
}/* dirsub*/



/*
 * Routine to ask for a direction and polymorph a monst 
 *
 * dirpoly()
 *
 * Subroutine to polymorph a monster and ask for the direction its in Enter with
 * the spell number in spmun. Returns no value. 
 */
static void
dirpoly() {
    int x, y, m;

    if (isconfuse()) return;     // if player is confused, can't aim magic

    if (dirsub(&x, &y) == DIR_CANCEL) return;

    if (Map[x][y].mon.id == 0) {
        say("There wasn't anything there!");
        return;
    }

    if (nospell(CPOLY, Map[x][y].mon.id)) {
        lasthit(x, y);
        return;
    }

    // There is a very small chance of this producing a genocided
    // creature, but that just makes things interesting.
    for (int n = 0; n < 100; n++) {
        m = rnd(DEMONKING-1);
        if ((MonType[m].flags & FL_GENOCIDED) == 0) { break; }
    }
    
    Map[x][y].mon = mk_mon(m);
    show1cell(x, y);    /* show the new monster */
}/* dirpoly */


/*
 * Routine to direct spell damage 1 square in 1 dir direct(spnum,dam,str,arg)
 * int spnum,dam,arg; char *str; 
 *
 * Routine to ask for a direction to a spell and then hit the monster Enter with
 * the spell number in spnum, the damage to be done in dam, lprintf format
 * string in str, and lprintf's argument in arg. Returns no value. 
 */
static void
direct(enum SPELL spell, int dam, char *str, int arg) {
    int x, y, mon_id;

    if (isconfuse()) return;

    if (dirsub(&x, &y) == DIR_CANCEL) return;

    mon_id = Map[x][y].mon.id;
    if (Map[x][y].obj.type == OMIRROR) {
        if (spell == CSLEEP || spell == CWEB) {
            say(spell == CSLEEP ? "You fall asleep! " 
                   : "You get stuck in your own web! ");
            headsup();
            arg += 2;
            while (arg-- > 0) {
                onemove(DIR_STAY, false);
                nap(1000);
            }
            return;
        } else {
            say(str, "spell caster (that's you)", (long) arg);
            headsup();
            losehp(dam, DDMAGICOOPS);
            return;
        }
    }// if
    
    if (mon_id == 0) {
        say("There wasn't anything there!");
        return;
    }// if
    
    if (nospell(spell, mon_id)) {
        lasthit(x, y);
        return;
    }// if
    
    say(str, monname_at(x, y), (long) arg);
    hitm(x, y, dam);
}/* direct*/



/* Display all ungenocided monsters and let the user pick one,
 * returning the index in MonType[].  -1 means cancel was pressed. */
static int
select_genmonst() {
    struct PickList *pl;
    int n, id = 0;
    bool selected;

    pl = pl_malloc();
    for (n = 1; n < MAXCREATURE; n++) {
        char buffer[200];

        if (MonType[n].flags & FL_GENOCIDED) continue;

        snprintf(buffer, sizeof(buffer), "'%c' %s", MonType[n].mapchar,
                 MonType[n].name);
        pl_add(pl, n, 0, buffer);
    }

    selected = pick_item(pl, "Genocide which monster?", &id);
    pl_free(pl);

    if (!selected) return -1;
    return id;
}/* select_genmonst*/


/*
 * Function to ask for monster and genocide from game.  This is done
 * by setting a flag in the monster[] structure
 */
void
genmonst() {
    int  i;

    i = select_genmonst();

    if (i < 0) {
        say("You relent.\n");
        return;
    }

    MonType[i].flags |= FL_GENOCIDED;  /* genocided from game */
    GS.spellknow[CGENOCIDE] = false;
    loseint();

    say("There will be no more %ss.\n", MonType[i].name);

    /* now wipe out monsters on this level */
    setlevel(getlevel());

    update_display(true);
}/* genmonst*/

