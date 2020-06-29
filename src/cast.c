// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "cast.h"

#include "internal_assert.h"

#include "display.h"
#include "sphere.h"
#include "movem.h"
#include "game.h"
#include "gender.h"
#include "ui.h"

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
static DIRECTION dirsub(int8_t *x, int8_t *y);
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

        if (known != UU.spellknow[n]) continue;

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

    if ((rnd(23) == 7) || (rnd(18) > intelligence())) {
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
        say("You feel safer.\n");
        UU.protectionTime += 250;
        return;

    case CMMISSILE: {    /* magic missile */
        int i = rnd(((UU.level + 1) << 1)) + UU.level + 3;
        godirect(spell, i, (UU.level >= 2) ?
                 "Your missiles hit the %s." :
                 "Your missile hit the %s.", 125, '+');
        return;
    }

    case CDEX:     /* dexterity     */
        say("You feel a little lighter on your feet.\n");
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
        say("You feel %s.\n", pretty(UU.gender));
        UU.charmcount += charisma() * 2;
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
        say("Everything feels a little lighter.\n");
        UU.strcount += 150 + rnd(100);
        return;

    case CENLIGHTEN:
        enlighten(10, rnd(3));
        return;

    case CHEALING:     /* healing */
        say("You feel better.\n");
        raisehp(20 + (UU.level << 1));
        return;

    case CCBLIND:        /* cure blindness    */
        say("%s\n", UU.blindCount  ?
            "You can see again." :
            "Your eyes feel rested but nothing else has changed.");
        UU.blindCount = 0;
        return;

    case CCREATEMON:
        createmonster(makemonst(getlevel() + 1) + 8);
        say("Poof!\n");
        return;

    case CPHANTASM:
        if (rnd(11) + 7 <= wisdom()) {
            direct(spell, rnd(20) + 20 + UU.level, "The %s believed!", 0);
        } else {
            say("It didn't believe the illusions!\n");
        }
        return;


    case CINV: {
        say("You %s yourself turning%s transparent.\n",
            UU.blindCount ? "feel" : "see",
            UU.invisibility ? " slightly more" : "");

        UU.invisibility += 12;

        // If they have the amulet of invisibility, then add more time.
        int idx = index_of_first(OAMULET);
        if (idx >= 0) {
            int extra = 1 + Invent[idx].iarg;
            UU.invisibility += extra << 7;
        }// if

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
        say("Colors fade for a brief moment.\n");
        UU.cancellation += 5 + UU.level;
        return;

    case CHASTE:        /* haste self    */
        say("The world seems to slow down.\n");
        UU.hasteSelf += 7 + UU.level;
        return;

    case CCLOUD:        /* cloud kill */
        omnidirect(spell, 30 + rnd(10), "The %s gasps for air!\n");
        return;

    case CVPROCK:        /* vaporize rock */
        say("The surrounding rock seems to melt.\n");
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

        // From iLarn:

        i = min(UU.hp - 1, UU.hpmax / 2);
        direct(spell, i + i, "A life-sapping force surrounds the %s,"
               " like a really bad sitcom.", 0);

        UU.hp -= i;
        return;
    }

    case CINVULN: {
        bool new_globe = UU.globe == 0;
        say("Your head hurts as %s transparent globe %s around you.\n",
            new_globe ? "a" : "the",
            new_globe ? "forms" : "strengthens");
        UU.globe += 200;
        loseint();  /* globe of invulnerability */
        return;
    }

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
        if (wisdom() > rnd(10) + 10)
            direct(spell, 2000,
                   "The %s's heart stopped.", 0);
        else
            say("It didn't work.\n");
        return;

        /* ----- LEVEL 5 SPELLS ----- */

    case CSCAREMON: {        /* scare monster */
        say("You feel incredibly badass!\n");
        UU.scaremonst += rnd(10) + UU.level;

        /* if have HANDofFEAR make last longer */
        if (has_a(OHANDofFEAR)) {
            UU.scaremonst *= 3;
        }/* if */
        return;
    }

    case CHOLDMON:        /* hold monster */
        say("The cave becomes eerily quiet.\n");
        UU.holdmonst += rnd(10) + UU.level;
        return;

    case CTIMESTOP:
        say("\"Time stand still...\"\n");
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
        int8_t xl, yl;

        DIRECTION dir = dirsub(&xl, &yl);   /* get direction of sphere */
        if (dir == DIR_CANCEL) return;

        if (rnd(23) == 5) {
            headsup();
            say("You have been enveloped by the zone of nothingness!\n\n");
            nap(4000);
            game_over_probably(DDSELFANNIH);
            return;
        }
        loseint();
        newsphere(&UU.spherelist, xl, yl, dir); /* make a sphere */
        return;
    }

    case CBANISH:        /* banish */
        banmonst();
        return;

    case CSUMMON: {       /* summon demon */
        int i;

        if (rnd(100) > 30) {
            direct(spell, 150, "The demon strikes at the %s.", 0);
            return;
        }
        if (rnd(100) > 15) {
            say("Nothing seems to have happened.\n");
            return;
        }
        say("The demon clawed you and vanished!\n");
        headsup();
        i = rnd(40) + 30;
        losehp(i, DDDEMON);  /* must say killed by a demon */
        return;
    }

    case CWALLWALK:        /* walk through walls */
        say("You feel phase-modulated.\n");
        UU.wtw += rnd(10) + 5;
        return;

    case CALTER_REALITY:        /* alter reality */
        alter_reality();
        return;

    case CPERMANENCE:        /* permanence */
        say("that ought to do it...\n");
        adjust_effect_timeouts(0, true);
        UU.spellknow[CPERMANENCE] = false;   /* forget */
        loseint();
        return;

    default:
        // This shouldn't be reachable, but...
        say("%d? You don't know that one.\n", (long) spell);
        headsup();
        return;
    }
}/* spelldamage*/

/*
 * Subtract 1 from your int (intelligence) if > 3
 */
static void
loseint() {
    intelligence_adjust(-1);
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
        say("You can't aim your magic!\n");
        headsup();
    }
    return (UU.confuse);
}

/*
 * Subroutine to return 1 if the spell can't affect the monster
 * otherwise returns 0. Enter with the spell number in 'spell', and
 * the monster number in monst.
 */
static int
nospell(enum SPELL spell, int monst) {
    static const char *spellEffects[LAST_MONSTER+1][SPNUM];
    static bool spellEffectsInitialized = false;
    const char *msg;

    /* bad spell or monst */
    ASSERT(spell < SPNUM && monst <= LAST_MONSTER && monst > 0 && spell >= 0);

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

                ASSERT (mon <= LAST_MONSTER && spell < SPNUM);
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
 */
void
godirect(enum SPELL spnum, int dam, char *str, int delay, char cshow) {
    int8_t x, y;
    int8_t dx, dy;

    if (isconfuse()) {
        return;
    }

    // Set dx and dy to increment.  We bail if it's 0,0 because that
    // means the player has tried to cast off the edge of the map.
    if (dirsub(&dx, &dy) == DIR_CANCEL) return;

    dx -= UU.x;
    dy -= UU.y;
    if (dx == 0 && dy == 0) return;


    x = UU.x;
    y = UU.y;

    while (dam > 0) {
        x += dx;
        y += dy;

        if (!inbounds(x, y)) {
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
            flash_at(x, y, cshow, delay);
        }/* if */

        struct Monster mon = at(x, y)->mon;
        if (mon.id != 0) {
            /* If there's a monster here... */
            flash_at(x, y, monchar(mon.id), delay);

            if (mon.id == DEMONKING || (mon.id >= DEMONLORD1 && rnd(100) < 10)){
                /* cannot cast a missile spell at the demon king!! */
                dx *= -1;
                dy *= -1;
                say("The %s returns your puny missile!\n", monname_mon(mon));
            } else {
                if (nospell(spnum, mon.id)) {
                    lasthit(x,y);
                    return;
                }
                say(str, monname_at(x, y));
                say("\n");
                dam -= hitm(x, y, dam, false);

                nap(1000);
                x -= dx;
                y -= dy;
            }// if .. else

        } else {

            /* If there's an object here, we may interact with it... */

            //it = &at(x, y)->obj.type;
            struct Object *it = &at(x, y)->obj;
            switch (it->type) {
            case OWALL:
                say(str, "wall");

                if (dam >= 50 + UU.challenge &&     /*enough damage?*/
                    getlevel() < VBOTTOM-2 && /* can't break wall below V2 */

                    // Within bounds?
                    x < MAXX - 1 && y < MAXY - 1 && x > 0 && y > 0)
                {
                    say(" The wall crumbles.\n");
                    *it = NULL_OBJ;
                }// if

                say("\n");
                dam = 0;
                break;

            case OCLOSEDDOOR:
                say(str, "door");
                if (dam >= 40) {
                    say(" The door is blasted apart.");
                    *it = NULL_OBJ;
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
                    say(" The statue crumbles.");
                    *it = obj(OBOOK, getlevel());
                }/* if */
                say("\n");
                dam = 0;
                break;

            case OTHRONE:
                say(str, "throne");
                if (dam > 33) {
                    at(x, y)->mon = mk_mon(GNOMEKING);
                    *it = obj(OTHRONE2, 0);
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

        // The missile lights the way...
        if (UU.blindCount == 0) {
            see_and_update_at(x, y);
        }// if

        dam -= 3 + (int) (UU.challenge >> 1);
    }// while
}/* godirect*/



/*
 * Routine to teleport away a monster.
 *
 * Routine to ask for a direction to a spell and then teleport a monster away.
 */
static void
tdirect(){
    int8_t x, y;
    uint8_t m;

    if (isconfuse()) return;

    if (dirsub(&x, &y) == DIR_CANCEL) return;
    if ((m = at(x, y)->mon.id) == 0) {
        say("There wasn't anything there!");
        return;
    }
    if (nospell(CTELEPORT, m)) {
        lasthit(x, y);
        return;
    }
    teleportmonst(x,y);
    say("Pop!\n");
}/* tdirect*/



/* Create a wall in the requested direction. */
static void
makewall() {
    int8_t x, y;

    if (isconfuse()) return;

    if (dirsub(&x, &y) == DIR_CANCEL) return;
    ASSERT(inbounds(x, y));

    // dirsub will clip to the map which can become the player's
    // location.  We don't allow that, even if it would be funny.
    if (x == UU.x && y == UU.y) { return; }

    /* can't make anything on walls */
    if (at(x, y)->obj.type == OWALL) {
        say("there's a wall there already!\n");
        return;
    }// if

    /* is it free of items? */
    if (at(x, y)->obj.type != 0) {
        say("there's something there already!\n");
        return;
    }// if

    /* is it free of monsters? */
    if (at(x, y)->mon.id != 0) {
        say("there's a monster there!\n");
        return;
    }// if

    // Okay, we can proceed
    at(x, y)->obj = obj(OWALL, 0);
    say("Rock appears out of thin air.\n");
}/* makewall*/


/* Perform the 'vaporize rock' spell */
static void
vaporizerock() {
    int xh = min(UU.x + 1, MAXX - 2);
    int yh = min(UU.y + 1, MAXY - 2);

    for (int x = max(UU.x - 1, 1); x <= xh; x++) {
        for (int y = max(UU.y - 1, 1); y <= yh; y++) {
            struct MapSquare *pt = at(x, y);

            switch (pt->obj.type) {
            case OWALL:
                /* can't vpr below V2 */
                if (getlevel() < VBOTTOM-2) {
                    pt->obj = NULL_OBJ;
                }/* if */
                break;

            case OSTATUE:
                if (UU.challenge > 3 && rnd(60) < 30) {
                    break;
                }/* if */
                pt->obj = obj (OBOOK, getlevel());
                break;

            case OTHRONE:
                pt->mon = mk_mon(GNOMEKING);
                pt->obj = obj(OTHRONE2, 0);
                break;

            case OALTAR:
                pt->mon = mk_mon(DEMONPRINCE);
                createmonster(DEMONPRINCE);
                createmonster(DEMONPRINCE);
                createmonster(DEMONPRINCE);
                createmonster(DEMONPRINCE);
                break;
            }/* switch */


            if (pt->mon.id == XORN || pt->mon.id == TROLL) {
                hitm(x, y, 200, false);
            }// if
        }/* for */
    }/* for */
}/* vaporizerock*/


// Perform the 'alter reality' spell (i.e. recreate the current map
// level while keeping the objects and creatures.
static void
alter_reality() {
    say("Suddenly, everything is different.  Or was it always that way?\n");
    remake_map_keeping_contents();

    loseint();
    force_full_update();
    update_display();

    UU.spellknow[CALTER_REALITY] = false;

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
            if ((m = at(x, y)->mon.id) != 0) {
                if (nospell(spell, m) == 0) {
                    say(str, monname_at(x, y));
                    hitm(x, y, dam, false);
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
dirsub(int8_t *x, int8_t *y) {
    DIRECTION dir;

    dir = promptdir(true);
    adjpoint(UU.x, UU.y, dir, x, y);
    clip(x, y);

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
    if (isconfuse()) return;     // if player is confused, can't aim magic

    int8_t x, y;
    if (dirsub(&x, &y) == DIR_CANCEL) return;

    if (at(x, y)->mon.id == 0) {
        say("There wasn't anything there!\n");
        return;
    }

    if (nospell(CPOLY, at(x, y)->mon.id)) {
        lasthit(x, y);
        return;
    }

    // There is a very small chance of this producing a banished
    // creature, but that just makes things interesting.
    uint8_t newmon;
    for (int n = 0; n < 100; n++) {
        newmon = rnd(DEMONKING-1);
        if (!is_banished(newmon)) { break; }
    }

    say("The air warps around the %s.\n", monname_at(x, y));
    at(x, y)->mon = mk_mon(newmon);
}/* dirpoly */


// Perform one of several (but not all!) directional spell attacks
// after prompting the player for a direction.
//
// Damage is determined by 'spell' and the damage amount by 'dam'.
// 'str' is the message to print and contains one or two format
// characters: a '%s' for the monster name followed by a '%d' for the
// number of hits.
//
// If count == 0, it is not displayed and 'str' should NOT have the
// '%d' format sequence.
static void
direct(enum SPELL spell, int dam, char *str, int count) {
    if (isconfuse()) return;

    int8_t x, y;
    if (dirsub(&x, &y) == DIR_CANCEL) return;

    uint8_t mon_id = at(x, y)->mon.id;
    if (at(x, y)->obj.type == OMIRROR) {
        if (spell == CSLEEP || spell == CWEB) {
            say(spell == CSLEEP         ?
                "You fall asleep! "     :
                "You get stuck in your own web! ");
            headsup();
            count += 2;
            while (count-- > 0) {
                onemove(DIR_STAY);
                nap(1000);
            }
            return;
        } else {
            ASSERT(count == 0);   // Currently, only sleep and web have a count.
            say(str, "spell caster (that's you)");
            say("\n");
            headsup();
            losehp(dam, DDMAGICOOPS);
            return;
        }
    }// if

    if (mon_id == 0) {
        say("There wasn't anything there!\n");
        return;
    }// if

    if (nospell(spell, mon_id)) {
        lasthit(x, y);
        return;
    }// if

    const char *mname = monname_at(x, y);
    if (count > 0) {
        say(str, mname, (long) count);
    } else {
        say(str, mname);
    }// if .. else

    say("\n");
    hitm(x, y, dam, false);
}/* direct*/



/* Display all unbanished monsters and let the user pick one,
 * returning the index in MonType[].  -1 means cancel was pressed. */
static int
select_banmonst() {
    struct PickList *pl;
    int id = 0;
    bool selected;

    pl = pl_malloc();
    for (enum MONSTER_ID n = 1; n < MAXCREATURE; n++) {
        char buffer[200];

        if (is_banished(n)) { continue; }

        snprintf(buffer, sizeof(buffer), "'%c' %s", monchar(n),
                 MonType[n].name);
        pl_add(pl, n, 0, buffer);
    }

    selected = pick_item(pl, "Banish which monster?", &id);
    pl_free(pl);

    if (!selected) return -1;
    return id;
}/* select_banmonst*/


/*
 * Function to ask for monster and banish from game.  This is done
 * by setting a flag in the monster[] structure
 */
void
banmonst() {
    int  i;

    i = select_banmonst();

    if (i < 0) {
        say("You relent.\n");
        return;
    }

    banish_monster(i);
    UU.spellknow[CBANISH] = false;
    loseint();

    say("All %ss are hereby banished from Larn.\n", MonType[i].name);

    setlevel(getlevel(), false);   // Level change removes banished monsters

    update_display();
}/* banmonst*/
