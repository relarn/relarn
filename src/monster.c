// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "internal_assert.h"
#include "monster.h"

#include "display.h"
#include "movem.h"
#include "show.h"
#include "game.h"
#include "ui.h"

#include "monster.h"

/* Create the big list of monster types: */
const struct MonstTypeData MonType[] = {
#define MONSTER(id,sym,lv,ac,dmg,attack,intl,gold,hp,exp,flags,longdesc) \
    {longdesc, sym, lv, ac, dmg, attack, intl, gold, hp, exp, flags},
#include "monster_list.h"
#undef MONSTER
};

static bool spattack(enum SP_ATTACK attack, int xx, int yy);
static void dropsomething (int x, int y, int mon_id);
static void rustattack(const char *monster);


// Return default monster hitpoints adjusted for challenge level
short
mon_hp(uint8_t id) {
    ASSERT(id <= LAST_MONSTER);
    return min(0xFFFF,  ((6 + UU.challenge) * MonType[id].hitpoints + 1) / 6);
}

// Return the experience gained from killing a monster of type 'id'
// adjusted for the challenge level.
int
mon_exp(uint8_t id) {
    ASSERT(id <= LAST_MONSTER);
    return max_l( 1, (7 * MonType[id].experience) / (7 + UU.challenge) + 1);
}// mon_hp

// Avoids pits
bool
avoidspits(struct Monster mon) {
    return MonType[mon.id].flags & FL_NOPIT;
}/* monflags*/


/* Test if mon can *not* be seen by the player. */
bool
cantsee(struct Monster mon) {
    return (isdemon(mon) && !UU.hasTheEyeOfLarn) ||
        (!UU.seeinvisible && (monflags(mon) & FL_INVISIBLE));
}/* return */


// Create a monster of type 'mon' next to the player.
void
createmonster(enum MONSTER_ID mon) {
    createmonster_near(mon, UU.x, UU.y);
}// createmonster

// Create a monster of type 'mon' NEXT TO the point at pos_x, pos_y.
void
createmonster_near(enum MONSTER_ID mon, int pos_x, int pos_y) {
    ASSERT(mon >= 1 && mon <= LAST_MONSTER);

    // Advance if banished
    while (is_banished(mon) && mon < MAXCREATURE) {
        mon++;
    }

    // We pick a direction at random and see if we can place a
    // creature there.  If not, we advance and try again, repeating
    // until all adjacent squares have been tried.
    //
    // Since advancing by 1 tends to bias for the first free square in
    // the search order if the player is mostly surrounded, we advance
    // by either 3 or 5.  (These are relatively prime to 8 and so will
    // cycle through all possible values.)
    int index = rund(8);
    int offset = rnd(1) ? 3 : 5;    // Needs to be prime wrt. num directions
    for (int i = 0; i < 8; i++) {
        DIRECTION dir = DIR_MIN_DIR + index;

        // Try direction 'dir'
        int8_t x = 0, y = 0;
        adjpoint(pos_x, pos_y, dir, &x, &y);

        // if we can create here, do so and quit
        if (cgood(x, y, 0, 1)) {
            at(x, y)->mon = mk_mon(mon);
            at(x, y)->mon.awake = (mon == ROTHE || mon == POLTERGEIST ||
                                   mon == VAMPIRE);
            return;
        }/* if */

        // Advance by an amount
        index = (index + offset) % 8;
    }/* for */
}// createmonster_near



// Return the maximum possible damage that can be done by 'rollval'
// hits.  This is used for attacks like "sleep" and "web" and is used
// as the upper limit of damage inflicted during a melee attack.
int
fullhit(int rollval) {
    int dmg;

    if (rollval < 0 || rollval > 20)
        return 0; /* fullhits are out of range */

    if (wielding(OLANCE))
        return 10000; /* lance of death */

    dmg = rollval * ((weaponclass() >> 1)
                     + strength()
                     - UU.challenge
                     - 12);

    return ((dmg >= 1) ? dmg : rollval);
}// fullhit


/* Return a string containing the name of the monster at x, y *unless*
 * the player is blind, in which case it's just the word "monster". */
const char *
monname_at(int x, int y) {
    if (!inbounds(x,y)) { return "monster"; }   // Probably can't happen.
    return monname(at(x, y)->mon.id);
}// monname_at


/* Return a string containing the name of the monster with the given
 * ID (must be valid) or the word "monster" if the player is blind. */
const char *
monname(uint8_t id) {
    ASSERT(id <= LAST_MONSTER);
    return UU.blindCount ? "monster" : MonType[id].name;
}// monname



/*
 * Function to hit a monster at the designated coordinates.
 *
 * This routine is used for a bash & slash type attack on a
 * monster. Enter with the coordinates of the monster in
 * (x,y).
 */
void
hit_mon_melee(int x, int y) {
    int damag;

    if (UU.timestop) { return; }     /* not if time stopped */

    if (!inbounds(x, y)) { return; } // This should be an assert.

    struct Monster monst = at(x, y)->mon;
    if (!ismon(monst)) { return; }

    bool lemming_instakill = monst.id == LEMMING && !annoying_lemmings();

    const char *mname = monname_mon(monst);

    int to_hit_max = max(-127, mon_ac(monst) - UU.challenge)
        + UU.level
        + dexterity()
        + weaponclass()/4 - 12
        - UU.challenge;

    /* need at least random chance to hit */
    bool didhit = false;
    if (lemming_instakill || rnd(20) < to_hit_max || rnd(71) < 5) {
        didhit = true;
        damag = fullhit(1);     // will always kill a lemming
        if (damag < 9999) { damag = rnd(damag) + 1; }
    }
    say("You %s the %s.\n", didhit ? "hit" : "missed", mname);

    // If the monster can dull weapons, handle this now.  If a weapon
    // can rust, its enchantment can go negative and the weapon will
    // disintegrate if it reaches -10.  Otherwise, enchantment is
    // reduced by 1 to a minimum of 0, after which no more damage is
    // done.
    if (didhit && isdulling(monst) && UU.wield > 0) {
        struct Object wld = Invent[UU.wield];
        const int RUST_MIN = -10;

        if ( (canrust(wld) && wld.iarg > RUST_MIN) || wld.iarg > 0) {
            say("Your weapon is dulled by the %s.\n", mname);
            headsup();
            --Invent[UU.wield].iarg;
        } else if (canrust(wld) && Invent[UU.wield].iarg <= RUST_MIN) {
            say("Your weapon disintegrates!\n");
            Invent[UU.wield] = obj(ONONE, 0);
            UU.wield = -1;
            didhit = false; /* Didn't hit after all... */
        }/* if */

    }/* if */

    if (didhit) {
        hitm(x, y, damag, true);
    }

    // Metamorphs polymorph to something fearsome when endangered
    if (monst.id == METAMORPH && at(x, y)->mon.hitp < 25 && at(x, y)->mon.hitp > 0) {
        at(x, y)->mon.id = BRONZEDRAGON + rund(9);
    }// if

    // And lemmings reproduce when scared.
    if (monst.id == LEMMING && !lemming_instakill) {
        if (rnd(1000) <= 400) {
            createmonster(LEMMING);
        }
    }// if
}/* hit_mon_melee*/



// Adjust damage amount if appropriate when performing a melee attack
// against 'monst'.
static int
melee_effects(int amt, bool *lance_vs_demon, struct Monster monst) {

    // The Vorpal Blade has a chance of beheading.
    if (UU.wield > 0 && Invent[UU.wield].type == OVORPAL && rnd(20) == 1 &&
        !(MonType[monst.id].flags & FL_NOBEHEAD)) {
        say("The Vorpal Blade goes snicker-snack and beheads the %s!\n",
            monname(monst.id));
        amt = monst.hitp;
    }// if 

    // Slayer and the Lance behave differently if the target is a demon
    if (isdemon(monst)) {
        if (wielding(OLANCE)) {
            amt = 300;
            *lance_vs_demon = true;
        }
        if (wielding(OSLAYER)) { amt = 10000; }
    }// if

    return amt;
}// melee_effects


/*
 * Function to just hit a monster at a given coordinates
 *
 * Returns the number of hitpoints the monster absorbed.  This routine
 * is used to specifically damage a monster at a location
 * (x,y). Called by hit_mon_melee(x,y) and other places.
 */
int
hitm(int x, int y, int amt, bool is_melee) {
    if(!inbounds(x, y)) { return 0; }   // This should be an ASSERT.

    int amt2 = amt;     /* save initial damage so we can return it */

    struct Monster *monst = &at(x, y)->mon;
    int mon_id = monst->id;
    const char *mname = monname(mon_id);

    /* if half damage curse adjust damage points */
    if (UU.halfdam) { amt >>= 1; }
    if (amt <= 0) { amt2 = amt = 1; }

    // Mark this monster as being angry
    lasthit(x, y);

    /* make sure hitting monst breaks stealth condition */
    at(x, y)->mon.awake = 1;
    UU.holdmonst = 0;   /* hit a monster breaks hold monster spell */

    /* if a dragon and orb(s) of dragon slaying  */
    if (has_a(OORBOFDRAGON) && isdragon(*monst)) {
        amt *= 3;
    }/* if */

    // Certain melee weapons do special things when used; do that here.
    bool lancemsg = false;
    if (is_melee) {
        amt = melee_effects(amt, &lancemsg, *monst);
    }// if 
    
    /* invincible monster fix is here */
    if (monst->hitp > mon_hp(mon_id)) {
        monst->hitp = mon_hp(mon_id);
    }// if

    // If the monster is killed...
    if (monst->hitp <= amt) {
        int16_t hpoints = monst->hitp;
        int mgold =
            min(0xFFFF, (10 * MonType[mon_id].gold) / (10 + UU.challenge) );

        // If this was the Big Bad, make a note of it for scoring.
        if (monst->id == DEMONKING) {
            UU.killedBigBad = true;
        }// if

        say("The %s died!\n", mname);
        raiseexperience(mon_exp(mon_id));
        *monst = NULL_MON;

        dropsomething(x, y, mon_id);

        if (mgold > 0) {
            // A bachelor's degree will increase your earnings by 40%
            // (according to my extremely shoddy research).  So stay
            // in school, kids!
            //
            // (We check for both the diploma and graduated() because
            // it's possible to have your degree revoked while still
            // keeping the diploma.)
            if (has_a(ODIPLOMA) && graduated()) {
                mgold += (double)mgold * 0.4 + 1;
            }/* if */

            dropgold(rnd(mgold) + mgold);
        }/* if */

        monst->hitp = 0;
        return hpoints;
    }// if

    if (lancemsg) {
        say("Your lance of death tickles the %s!\n", mname);
    }// if

    monst->hitp -= amt;
    return amt2;
}/* hitm */


/*
 *  Function for the monster to hit the player from (x,y)
 */
void
hitplayer (const int x, const int y) {
    int dam,tmp,mster,bias;
    const char *mname;

    if (!inbounds(x, y)) { return; }    // This should be an assert.

    mster = at(x, y)->mon.id;
    mname = monname(mster);

    bias = UU.challenge + 1;

    if (mster == LEMMING) {
        return;
    }

    if (mster < DEMONLORD1)
        if (UU.invisibility && rnd(33)<20) {
            say("The %s misses wildly!\n",mname);
            return;
        }

    // If the player has Charm Monster and the monster isn't immune,
    // the monster may be charmed out of the attack.
    if (UU.charmcount               &&
        mster < DEMONLORD1          &&
        mster != PLATINUMDRAGON     &&
        rnd(30) + 5*MonType[mster].level - charisma() < 30)
    {
        say("The %s is awestruck by your magnificence!\n", mname);
        return;
    }

    // Base damage, adjusted for challenge level
    dam = min( 127, ((6 + UU.challenge) * MonType[mster].damage + 1)/5 );

    // Add damage roll
    dam += rnd(dam < 1 ? 1 : dam) + MonType[mster].level;

    /* demon lords/prince/god of hellfire damage is reduced if wielding
       Slayer */
    if (mster >= DEMONLORD1)
        if (Invent[UU.wield].type==OSLAYER)
            dam=(int) (1 - (0.1 * rnd(5)) * dam);

    /* spirit naga's and poltergeist's damage is halved if scarab of
      negate spirit */
    if (
        (mster == POLTERGEIST || mster == SPIRITNAGA )
        &&
        (has_a(OSPIRITSCARAB) || UU.spiritpro)
        )
    {
        dam = (int) dam/2;
    }

    /*  halved if undead and cube of undead control */
    if (has_a(OCUBE_of_UNDEAD) || UU.undeadpro)
        if ((mster ==VAMPIRE) || (mster ==WRAITH) || (mster ==ZOMBIE))
            dam = (int) dam/2;

    tmp = 0;
    if (MonType[mster].attack)
        if (((dam + bias + 8) > defense())
            || (rnd((int)((defense()>0)?defense():1))==1)) {
            if (spattack(MonType[mster].attack, x, y)) {
                return;
            }
            tmp = 1;
            bias -= 2;
        }

    if (((dam + bias) > defense()) || (rnd((int)((defense()>0)?defense():1))==1)) {
        say("The %s hit you.\n", mname);
        tmp = 1;
        if ((dam -= defense()) < 0)
            dam=0;

        if (dam > 0) {
            losehp(dam, mster);
        }
    }

    if (tmp == 0) {
        say("The %s missed.\n",mname);
    }
}/* hitplayer */

/* Create and place the loot that gets dropped when a monster is
 * killed. */
static void
dropsomething (int x, int y, int mon_id) {

    /* If this monster has a steal attack and there are stolen goods
     * on this level, return some. */
    if (lev()->numStolen && (MonType[mon_id].attack == SA_STEALGOLD ||
                           MonType[mon_id].attack == SA_STEAL ||
                           MonType[mon_id].attack == SA_MULTI)   ) {
        int i;

        for (i = 0; i < rnd(3); i++) {
            struct Object sob;

            sob = remove_stolen(lev());
            if (!sob.type) {
                break;
            }/* if */

            createitem(x, y, sob);
        }/* for */

        return;
    }/* if */


    /* Otherwise, drop a random item if the monster is up for it. */
    switch(mon_id) {
    case ORC:
    case NYMPH:
    case ELF:
    case TROGLODYTE:
    case TROLL:
    case ROTHE:
    case VIOLETFUNGI:
    case PLATINUMDRAGON:
    case GNOMEKING:
    case REDDRAGON:
        create_rnd_item(x,y,getlevel());
        break;

    case LEPRECHAUN:
        if (rnd(101)>=75) creategem();
        if (rnd(5)==1) dropsomething(x,y,LEPRECHAUN);
        break;
    }/* switch*/
}/* dropsomething */


/*
 * Function to drop some gold around player.  Enter with the number of
 * gold pieces to drop.
 */
void
dropgold(int amount) {
    ASSERT (amount < (1 << 24));

    if (amount < 1) {
        return;
    }/* if */

    createitem(UU.x, UU.y, obj(OGOLDPILE, (unsigned) amount));
}/* dropgold*/



/*
 * Function to process special attacks from monsters.
 *
 * Enter with the special attack ID and the coordinates (xx,yy) of the
 * monster that is special attacking.  Returns true if this is the
 * monster's only attack (currently only true for stealing); false if
 * there may also be conventional attackes.
 *
 * (This result is probably no longer useful.)
 *
 * atckno           monster             effect
 * ---------------------------------------------------
 * SA_NONE          none
 * SA_RUST          rust                eat armor
 * SA_FIRE          hell hound          breathe light fire
 * SA_BIGFIRE       dragon              breathe fire
 * SA_STING         giant centipede     weakening strength
 * SA_COLD          white dragon        cold breath
 * SA_DRAIN         wraith              drain level
 * SA_GUSHER        waterlord           water gusher
 * SA_STEALGOLD     leprechaun          steal gold
 * SA_DISENCHANT    disenchantress      disenchant weapon or armor
 * SA_TAILTHWACK    ice lizard          hits with barbed tail
 * SA_CONFUSE       umber hulk          confusion
 * SA_MULTI         spirit naga         cast spells taken from special attacks
 * SA_PSIONICS      platinum dragon     psionics
 * SA_STEAL         nymph               steal objects
 * SA_BITE          bugbear             bite
 * SA_BIGBITE       osequip             bite more
 *
 */
static bool
spattack(enum SP_ATTACK attack, const int xx, const int yy) {
    int i;
    bool is_lesser_attack = false;

    static char spsel[] = {SA_RUST, SA_FIRE, SA_BIGFIRE, SA_COLD, SA_DRAIN,
                           SA_STEALGOLD, SA_DISENCHANT, SA_CONFUSE, SA_PSIONICS,
                           SA_STEAL};


    ASSERT(inbounds(xx, yy));

    const uint8_t monst = at(xx, yy)->mon.id;
    const char *mname = monname(monst);

    /*
     * cancel only works 5% of time for demon prince and god
     */
    if (UU.cancellation) {
        if (monst >= DEMONPRINCE) {
            if (rnd(100) >= 95)
                return false;
        } else
            return false;
    }/* if */

    /* staff of power cancels demonlords/wraiths/vampires 75% of time */
    /* the demon king is unaffected */
    if (monst != DEMONKING) {
        if ((monst >= DEMONLORD1) ||
            (monst == WRAITH) ||
            (monst == VAMPIRE))
            if (has_a(OPSTAFF))
                if (rnd(100) < 75)
                    return false;
    }/* if */

    /* if have cube of undead control,  wraiths and vampires do nothing */
    if ((monst == WRAITH) || (monst == VAMPIRE))
        if (has_a(OCUBE_of_UNDEAD) || (UU.undeadpro))
            return false;

    char *p = NULL;
    switch (attack) {
    case SA_NONE: return false; /* Shouldn't happen; shuts up gcc. */

    case SA_RUST:
        rustattack(mname);
        break;

    case SA_FIRE:
        i = rnd(15) + 8 - defense();
        is_lesser_attack = true;
        /* Fall through */

    case SA_BIGFIRE:
        if (!is_lesser_attack) i = rnd(20) + 25 - defense();

        say ("The %s breathes fire at you!\n", mname);
        if (UU.fireresistance) {
            if (rnd(15) != 7) {
                say ("The %s's flame doesn't faze you!\n", mname);
            } else {
                say ("You toast some marshmallows.\n");
            }/* if .. else*/
            i = 0;
        }/* if */

        losehp(i, monst);
        return false;

    case SA_STING:
        if (strength() > 3) {
            p = "The %s stung you!  You feel weaker.\n";
            headsup();
            strength_adjust(-1);
        } else {
            p = "The %s stung you!\n";
        }
        break;

    case SA_COLD:
        i = rnd(15) + 18 - defense();

        say("The %s blasts you with its cold breath.\n", mname);
        headsup();

        losehp(i, monst);
        return false;

    case SA_DRAIN:
        say("The %s drains you of your life energy!\n", mname);
        loselevel();
        if (monst == DEMONPRINCE)
            losemspells(1);
        if (monst == DEMONKING) {
            loselevel();
            losemspells(2);
            raise_min(3);
        }
        headsup();
        return false;

    case SA_GUSHER:
        i = rnd(15) + 25 - defense();
        say("The %s got you with a gusher!\n", mname);
        headsup();

        losehp(i, monst);
        return false;

    case SA_STEALGOLD: {
        if (has_a(ONOTHEFT)) return false;

        if (UU.gold) {
            unsigned long stolen;

            stolen = (UU.gold > 32767) ? UU.gold/2 : rnd((int)(1 + UU.gold/2));
            stolen = (stolen > UU.gold) ? UU.gold : stolen;
            stolen = (stolen > MAX_IARG) ? MAX_IARG : stolen;

            add_to_stolen(obj(OGOLDPILE, stolen));

            UU.gold -= stolen;

            p = "The %s hit you.  Your purse feels lighter.\n";
        } else {
            p = "The %s couldn't find any gold to steal.\n";
        }/* if .. else*/

        say(p, mname);
        teleportmonst(xx, yy);

        headsup();

        return true;
    }

    case SA_DISENCHANT: {
        int tries = 50;

        for (;;) {
            int index = rund(IVENSIZE);
            struct Object target = Invent[index];

            if (--tries < 0) {
                p = "The %s nearly misses.\n";
                break;
            }/* if */

            /* Skip this one if it's not eligible for disenchantment. */
            if (!target.type || target.iarg == 0 || isscroll(target)
                || ispotion(target)) {
                continue;
            }/* if */

            Invent[index].iarg -= 3;
            if (Invent[index].iarg < 0) {
                Invent[index].iarg = 0;
            }/* if */

            say("The %s hits you with a spell of disenchantment! \n",
                    mname);
            headsup();
            show_inv_item(index);
            return false;
        }
        break;
    }

    case SA_TAILTHWACK:
        i = rnd(25) - defense();

        say("The %s hit you with its barbed tail.\n", mname);
        headsup();

        losehp(i, monst);
        return false;

    case SA_CONFUSE:
        UU.confuse += 10 + rnd(10);
        say ("The %s has confused you.\n", mname);
        headsup();
        break;

    case SA_MULTI:  /* performs any number of other special
                     * attacks   */
        return spattack(spsel[rund(10)], xx, yy);

    case SA_PSIONICS:
        p = "The %s flattens you with it's psionics!\n";
        i = rnd(15) + 30 - defense();

        say(p, mname);
        headsup();

        losehp(i, monst);
        return false;

    case SA_STEAL:
        if (has_a(ONOTHEFT)) {
            return false; /* players has device of no theft */
        }/* if */

        if (emptyhanded()) {
            p = "The %s couldn't find anything to steal.\n";
            break;
        }
        say("The %s picks your pocket and takes:\n", mname);
        headsup();

        if (stealsomething(xx,yy) == 0) {
            say("nothing");
        }/* if */

        teleportmonst(xx, yy);
        return true;

    case SA_BITE:
        i = rnd(10) + 5 - defense();
        is_lesser_attack = true;
        /* Fall through */

    case SA_BIGBITE:
        if (!is_lesser_attack) i = rnd(15) + 10 - defense();
        p = "The %s bit you!\n";

        say(p, mname);
        headsup();

        losehp(i, monst);
        return false;
    }/* switch */

    if (p) {
        say(p, mname);
    }
    return false;
}/* spattack*/


static void
rustattack(const char *monster) {
    bool rusted = false;
    int rustable = UU.wear;
    int maxrust;

    /* Rust the shield if wielded and rustable. */
    if (UU.shield > 0 &&
        Invent[UU.shield].iarg > -Types[OSHIELD].maxrust) {
        rustable = UU.shield;
    }/* if */

    /* Quit if not wearing armor or a shield. */
    if (rustable < 0) return;

    /* Rust the thing if possible. A 0 maxrust means it's rustproof.*/
    maxrust = -Types[ Invent[rustable].type ].maxrust;
    if (maxrust < 0 && Invent[rustable].iarg > maxrust) {
        --Invent[rustable].iarg;
        rusted = true;
    }/* if */

    /* And notify the player. */
    if (rusted) headsup();
    say("The %s hit you -- %s.\n", monster,
            rusted ? "your armor feels weaker." : "your armor is unaffected");
}/* rustattack*/


/*
 * Routine to annihilate all monsters around player (UU.x,UU.y)
 *
 * Gives player experience, but no dropped objects. Returns the
 * experience gained from all monsters killed
 */
void
annihilate() {
    int i, j;
    long k;
    uint8_t *p;

    for (k = 0, i = UU.x - 1; i <= UU.x + 1; i++) {
        for (j = UU.y - 1; j <= UU.y + 1; j++) {
            if (! inbounds(i, j)) { continue; }

            if (*(p = &at(i, j)->mon.id)) {  /* if a monster there */
                if (*p < DEMONLORD1) {
                    k += mon_exp(*p);
                    *p = 0;
                } else {
                    say("The %s barely escapes being annihilated!\n",
                        MonType[*p].name);
                    /* lose half hit points */
                    at(i, j)->mon.hitp = (at(i, j)->mon.hitp >> 1) + 1;
                }/* if .. else*/
            }/* if */
        }/* for */
    }/* for */

    if (k > 0) {
        say("You hear loud screams of agony!\n");
        raiseexperience((long) k);
    }
}/* annihilate*/

/*
 * function to return monster number for a randomly selected monster for the
 * given cave level
 */
int
makemonst(int lev) {
    static const char monstlevel[] = {5, 11, 17, 22, 27, 33, 39, 42, 46, 50,
                                      53, 56};
    int monst, x;

    if (lev < 1)
        lev = 1;
    if (lev > 12)
        lev = 12;

    monst = WATERLORD;

    if (lev < 5) {
        while (monst == WATERLORD) {
            monst = rnd(((x = monstlevel[lev - 1]) != 0) ? x : 1);
        }
    }
    else {
        while (monst == WATERLORD) {
            monst = rnd(((x=monstlevel[lev-1] - monstlevel[lev-4]) != 0) ? x : 1)
                + monstlevel[lev - 4];
        }
    }

    while (is_banished(monst) && monst < MAXCREATURE) {
        monst++;
    }

    if (getlevel() <= DBOTTOM) {
        if (rnd(100) < 10) {
            monst = LEMMING;
        }
    }

    return monst;
}/* makemonst*/



/*
  subroutine to randomly create monsters if needed
*/
void
randmonst () {
    int level;

    /*  don't make monsters if time is stopped  */
    if (UU.timestop)
        return;

    level = getlevel();

    /* No monsters in the town. */
    if (level == 0) return;

    if (--UU.monstCount <= 0) {
        UU.monstCount = 120 - level*4;
        fillmonst(makemonst(getlevel()));
    }/* if */
}/* randmonst */


// Place monster with ID `what` at a suitable location on the current map.  Return
int
fillmonst (int what) {

    /* max # of creation attempts */
    for (int trys = 10; trys > 0; --trys) {
        int x = rnd(MAXX-2);
        int y = rnd(MAXY-2);
        if (at(x, y)->obj.type == 0 && at(x, y)->mon.id == 0 &&
            (UU.x != x || UU.y != y))
        {
            at(x, y)->mon = mk_mon(what);
            return 0;
        }// if
    }// for

    return -1; /* creation failure */
}/* fillmonst */
