// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "internal_assert.h"
#include "monster.h"

#include "display.h"
#include "movem.h"
#include "show.h"
#include "game.h"

#include "monster.h"

/* Create the big list of monster types: */
struct MonstTypeData MonType[] = {
#define MONSTER(id,sym,lv,ac,dmg,attack,intl,gold,hp,exp,flags,longdesc) \
    {longdesc, sym, lv, ac, dmg, attack, intl, gold, hp, exp, flags},
#include "monster_list.h"
#undef MONSTER
};

const struct Monster NullMon = {0, 0, 0};

static int spattack(enum SP_ATTACK attack, int xx, int yy);
static bool verifyxy(int *x, int *y);
static void dropsomething (int x, int y, int mon_id);
static void rustattack(const char *monster);



bool
avoidspits(struct Monster mon) {
    return MonType[mon.id].flags & FL_NOPIT;
}/* monflags*/


/* Test if mon can *not* be seen by the player. */
bool
cantsee(struct Monster mon) {
    return (isdemon(mon) && !UU.eyeOfLarn) || 
        (!UU.seeinvisible && (monflags(mon) & FL_INVISIBLE));
}/* return */


/* wipe out a monster at a location */
void
disappear(int x, int y) {
    Map[x][y].mon.id = 0;
    Map[x][y].know = 0;
}/* disappear*/


// Create a monster of type 'mon' next to the player.
void
createmonster(int mon) {
    createmonster_near(mon, UU.x, UU.y);
}// createmonster

// Create a monster of type 'mon' NEXT TO the point at pos_x, pos_y.
void
createmonster_near(int mon, int pos_x, int pos_y) {
    int k, i;

    if (mon < 1 || mon > NUM_MONSTERS) {    // this should really be an ASSERT
        headsup();
        say("can't createmonst(%d)\n\n", (long) mon);
        nap(3000);
        return;
    }
    
    while ((MonType[mon].flags & FL_GENOCIDED) != 0 && mon < MAXCREATURE)
        mon++;      /* genocided? */
    
    /* choose direction, then try all */
    for (k = rnd(8), i = -8; i < 0; i++, k++) {
        if (k > 8) { k = 1; }  /* wraparound the diroff arrays */

        int x, y;
        adjpoint(pos_x, pos_y, k, &x, &y);
        
        /* if we can create here */
        if (cgood(x, y, 0, 1)) {
            Map[x][y].mon = mk_mon(mon);
            Map[x][y].know = 0;
            Map[x][y].mon.awake = (mon == ROTHE || mon == POLTERGEIST ||
                                   mon == VAMPIRE);
            return;
        }/* if */
    }/* for */
}// createmonster_at



// Return the maximum possible damage that can be done by 'rollval'
// hits.  This is used for attacks like "sleep" and "web" and is used
// as the upper limit of damage inflicted during a melee attack.
int
fullhit(int rollval) {
    int dmg;

    if (rollval < 0 || rollval > 20)
        return 0; /* fullhits are out of range */

    if (lancedeath())
        return 10000; /* lance of death */

    dmg = rollval * (
        (UU.wclass >> 1)
                     + UU.strength
                     + UU.strextra
                     - UU.challenge
                     - 12
                     + UU.moreDmg);

    return ((dmg >= 1) ? dmg : rollval);
}// fullhit


/* Return a string containing the name of the monster at x, y *unless*
 * the player is blind, in which case it's just the word "monster". */
const char *
monname_at(int x, int y) {
    VXY(x, y);      /* verify correct x,y coordinates */
    return monname(Map[x][y].mon.id);
}// monname_at


/* Return a string containing the name of the monster with the given
 * ID (must be valid) or the word "monster" if the player is blind. */
const char *
monname(uint8_t id) {
    ASSERT(id <= NUM_MONSTERS);
    // XXX Should we be doing invisibility checking here too?
    return UU.blindCount ? "monster" : MonType[id].name;
}// monname





/*
 * Routine to verify/fix coordinates for being within bounds 
 *
 * Function to verify x & y are within the bounds for a level If *x or *y is not
 * within the absolute bounds for a level, fix them so that they are on the
 * level. Returns true if it was out of bounds, and the *x & *y in the
 * calling routine are affected.
 *
 * TODO: merge this and the VXY macro.
 */
static bool
verifyxy(int *x, int *y) {
    bool changed = false;

    if (*x < 0) {
        *x = 0;
        changed = true;
    }
    if (*y < 0) {
        *y = 0;
        changed = true;
    }
    if (*x >= MAXX) {
        *x = MAXX - 1;
        changed = true;
    }
    if (*y >= MAXY) {
        *y = MAXY - 1;
        changed = true;
    }
    return changed;
}/* verifyxy*/



/*
 * Function to hit a monster at the designated coordinates.
 *
 * This routine is used for a bash & slash type attack on a
 * monster. Enter with the coordinates of the monster in
 * (x,y).
 */
void
hitmonster(int x, int y) {
    int monst, damag, didhit;
    const char *mname;

    if (UU.timestop)
        return;     /* not if time stopped */

    VXY(x, y);      /* verify coordinates are within range */

    if ((monst = Map[x][y].mon.id) == 0)
        return;

    mname = monname_at(x,y);

    int to_hit_max = max( -127, MonType[monst].armorclass - UU.challenge )
        + UU.level
        + UU.dexterity
        + UU.wclass / 4 - 12
        - UU.challenge;

    /* need at least random chance to hit */
    if ((rnd(20) < to_hit_max) || (rnd(71) < 5)) {
        didhit = 1;
        damag = fullhit(1);
        if (damag < 9999)
            damag = rnd(damag) + 1;
    } else {
        didhit = 0;
    }
    say("You %s the %s.\n", didhit ? "hit" : "missed", mname);

    /*
     *  If the monster was hit, deal with weapon dulling.
     */
    if (didhit && (monst==RUSTMONSTER || monst==DISENCHANTRESS || monst==CUBE)
        && UU.wield > 0) {
        /* if it's not already dulled to hell */
        if (((Invent[UU.wield].iarg > -10) &&
             ((Invent[UU.wield].type == OSLAYER) ||
              (Invent[UU.wield].type == ODAGGER) ||
              (Invent[UU.wield].type == OSPEAR) ||
              (Invent[UU.wield].type == OFLAIL) ||
              (Invent[UU.wield].type == OBATTLEAXE) ||
              (Invent[UU.wield].type == OLONGSWORD) ||
              (Invent[UU.wield].type == O2SWORD) ||
              (Invent[UU.wield].type == OLANCE) ||
              (Invent[UU.wield].type == OHAMMER) ||
              (Invent[UU.wield].type == OVORPAL) ||
              (Invent[UU.wield].type == OBELT)))
            || (Invent[UU.wield].iarg > 0)) {
            say("Your weapon is dulled by the %s.\n", mname);
            headsup();
            --Invent[UU.wield].iarg;
        } else if (Invent[UU.wield].iarg <= -10) {
            say("Your weapon disintegrates!\n");
            Invent[UU.wield] = obj(ONONE, 0);
            UU.wield = -1;
            didhit = 0; /* Didn't hit after all... */
        }/* if */
    }/* if */

    if (didhit) {
        hitm(x, y, damag);
        if ((monst >= DEMONLORD1) && lancedeath() && (Map[x][y].mon.hitp))
            say("Your lance of death tickles the %s!\n", mname);
    }
    if (monst == METAMORPH)
        if (Map[x][y].mon.hitp < 25 && Map[x][y].mon.hitp > 0) {
            Map[x][y].mon.id = BRONZEDRAGON + rund(9);
            show1cell(x, y);
        }
    if (Map[x][y].mon.id == LEMMING)
        if (rnd(100) <= 40)
            createmonster(LEMMING);
}/* hitmonster*/




/*
 * Function to just hit a monster at a given coordinates 
 *
 * Returns the number of hitpoints the monster absorbed.  This routine
 * is used to specifically damage a monster at a location
 * (x,y). Called by hitmonster(x,y) and other places.
 */
int
hitm(int x, int y, int amt) {
    int mon_id;
    struct Monster *monst;
    int amt2;
    const char *mname;

    VXY(x, y);      /* verify coordinates are within range */
    amt2 = amt;     /* save initial damage so we can return it */

    monst = &Map[x][y].mon;
    mon_id = monst->id;
    mname = monname(mon_id);

    /* if half damage curse adjust damage points */
    if (UU.halfdam)
        amt >>= 1;
    if (amt <= 0)
        amt2 = amt = 1;

    lasthit(x, y);

    /* make sure hitting monst breaks stealth condition */
    Map[x][y].mon.awake = 1;
    UU.holdmonst = 0;   /* hit a monster breaks hold monster spell */

    /* if a dragon and orb(s) of dragon slaying  */
    if (UU.slaying) {
        switch (mon_id) {
        case WHITEDRAGON:
        case REDDRAGON:
        case GREENDRAGON:
        case BRONZEDRAGON:
        case PLATINUMDRAGON:
        case SILVERDRAGON:
            amt *= 3;
            break;
        }
    }/* if */

    /* Deal with Vorpy */
    if (UU.wield > 0 && Invent[UU.wield].type == OVORPAL && rnd(20) == 1 &&
        !(MonType[mon_id].flags & FL_NOBEHEAD)) {
        say("The Vorpal Blade goes snicker-snack and beheads the %s!\n",mname);
        amt = monst->hitp;
    }

    /* invincible monster fix is here */
    if (monst->hitp > mon_hp(mon_id)) {
        monst->hitp = mon_hp(mon_id);
    }// if

    if (mon_id >= DEMONLORD1) {
        if (lancedeath())
            amt = 300;
        if (Invent[UU.wield].type == OSLAYER)
            amt = 10000;
    }

    if (monst->hitp <= amt) {
        short hpoints = monst->hitp;
        int mgold =
            min(0xFFFF, (10 * MonType[mon_id].gold) / (10 + UU.challenge) );
        

        say("The %s died!\n", mname);
        raiseexperience(mon_exp(mon_id));
        disappear(x, y);

        dropsomething(x, y, mon_id);

        if (mgold > 0) {
            /* An education will increase your earnings. */
            if (has_a(ODIPLOMA)) {
                mgold += (double)mgold / 10 + 1;
            }/* if */
            
            dropgold(rnd(mgold) + mgold);
        }/* if */

        show1cell(x,y);
        showplayerarea();

        monst->hitp = 0;
        return (hpoints);
    }

    monst->hitp -= amt;
    return (amt2);
}/* hitm */


/*
 *  Function for the monster to hit the player from (x,y)
 */
void
hitplayer (int x, int y) {
    int dam,tmp,mster,bias;
    const char *mname;

    VXY(x,y);   /* ensure coordinates are within range */

    mster = Map[x][y].mon.id;
    mname = monname(mster);

    if ((Map[x][y].know&1) == 0) {
        Map[x][y].know=1; 
        show1cell(x,y);
    }

    bias = UU.challenge + 1;

    if (mster==LEMMING) 
        return;

    if (mster < DEMONLORD1)
        if (UU.invisibility && rnd(33)<20) {
            say("The %s misses wildly!\n",mname);   
            return;
        }

    if (   mster < DEMONLORD1
        && mster != PLATINUMDRAGON
        && UU.charmcount
        && rnd(30) + 5*MonType[mster].level - UU.charisma < 30) {

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

    /*  spirit naga's and poltergeist's damage is halved if scarab of 
      negate spirit */
    if (UU.negatespirit || UU.spiritpro)  
        if ((mster ==POLTERGEIST) || (mster ==SPIRITNAGA)) 
            dam = (int) dam/2; 

    /*  halved if undead and cube of undead control */
    if (UU.cube_of_undead || UU.undeadpro) 
        if ((mster ==VAMPIRE) || (mster ==WRAITH) || (mster ==ZOMBIE)) 
            dam = (int) dam/2;

    tmp = 0;
    if (MonType[mster].attack)
        if (((dam + bias + 8) > UU.ac) 
            || (rnd((int)((UU.ac>0)?UU.ac:1))==1)) {    
            if (spattack(MonType[mster].attack, x, y)) {  
                return; 
            }
            tmp = 1;  
            bias -= 2; 
        }

    if (((dam + bias) > UU.ac) || (rnd((int)((UU.ac>0)?UU.ac:1))==1)) {
        say("The %s hit you.\n", mname);   
        tmp = 1;
        if ((dam -= UU.ac) < 0) 
            dam=0;

        if (dam > 0) { 
            losehp(dam, mster);
        }
    }

    if (tmp == 0)  
        say("The %s missed.\n",mname);
}/* hitplayer */

/* Create and place the loot that gets dropped when a monster is
 * killed. */
static void
dropsomething (int x, int y, int mon_id) {

    /* If this monster has a steal attack and there are stolen goods
     * on this level, return some. */
    if (Lev->numStolen && (MonType[mon_id].attack == SA_STEALGOLD ||
                           MonType[mon_id].attack == SA_STEAL ||
                           MonType[mon_id].attack == SA_MULTI)   ) {
        int i;

        for (i = 0; i < rnd(3); i++) {
            struct Object sob;
            
            sob = remove_stolen(Lev);
            if (!sob.type) {
                break;
            }/* if */
            
            createitem(x, y, sob.type, sob.iarg);
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
        something(x,y,getlevel());
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

    createitem(UU.x, UU.y, OGOLDPILE, (long) amount);
}/* dropgold*/



/*
 * Function to process special attacks from monsters.
 *
 * Enter with the special attack ID and the coordinates (xx,yy) of the
 * monster that is special attacking.  Returns 1 if must do a
 * show1cell(xx,yy) upon return, 0 otherwise.
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
static int 
spattack(enum SP_ATTACK attack, int xx, int yy) {
    int i;
    char *p = 0;
    bool is_lesser_attack = false;
    uint8_t monst;
    const char *mname;

    static char spsel[] = {SA_RUST, SA_FIRE, SA_BIGFIRE, SA_COLD, SA_DRAIN, 
                           SA_STEALGOLD, SA_DISENCHANT, SA_CONFUSE, SA_PSIONICS,
                           SA_STEAL};


    VXY(xx, yy);        /* verify x & y coordinates */

    monst = Map[xx][yy].mon.id;
    mname = monname(monst);

    /*
     * cancel only works 5% of time for demon prince and god 
     */
    if (UU.cancellation) {
        if (monst >= DEMONPRINCE) {
            if (rnd(100) >= 95)
                return (0);
        } else
            return (0);
    }/* if */

    /* staff of power cancels demonlords/wraiths/vampires 75% of time */
    /* the demon king is unaffected */
    if (monst != DEMONKING) {
        if ((monst >= DEMONLORD1) ||
            (monst == WRAITH) ||
            (monst == VAMPIRE))
            if (has_a(OPSTAFF))
                if (rnd(100) < 75)
                    return (0);
    }/* if */

    /* if have cube of undead control,  wraiths and vampires do nothing */
    if ((monst == WRAITH) || (monst == VAMPIRE))
        if ((UU.cube_of_undead) || (UU.undeadpro))
            return (0);

    switch (attack) {
    case SA_NONE: return 0; /* Shouldn't happen; shuts up gcc. */

    case SA_RUST:
        rustattack(mname);
        break;

    case SA_FIRE: 
        i = rnd(15) + 8 - UU.ac;
        is_lesser_attack = true;
        /* Fall through */

    case SA_BIGFIRE:
        if (!is_lesser_attack) i = rnd(20) + 25 - UU.ac;

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
        return (0);

    case SA_STING:
        if (UU.strength > 3) {
            p = "The %s stung you!  You feel weaker.\n";
            headsup();
            if (--UU.strength < 3)
                UU.strength = 3;
        } else
            p = "The %s stung you!\n";
        break;

    case SA_COLD:
        i = rnd(15) + 18 - UU.ac;

        say("The %s blasts you with his cold breath.\n", mname);
        headsup();

        losehp(i, monst);
        return (0);

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
        return (0);

    case SA_GUSHER:
        i = rnd(15) + 25 - UU.ac;
        say("The %s got you with a gusher!\n", mname);
        headsup();

        losehp(i, monst);
        return (0);

    case SA_STEALGOLD: {
        if (UU.notheft) return (0);

        if (UU.gold) {
            unsigned long stolen;

            stolen = (UU.gold > 32767) ? UU.gold/2 : rnd((int)(1 + UU.gold/2));
            stolen = (stolen > UU.gold) ? UU.gold : stolen;
            stolen = (stolen > MAX_IARG) ? MAX_IARG : stolen;

            add_to_stolen(obj(OGOLDPILE, stolen), Lev);

            UU.gold -= stolen;

            p = "The %s hit you.  Your purse feels lighter.\n";
        } else {
            p = "The %s couldn't find any gold to steal.\n";
        }/* if .. else*/

        say(p, mname);
        teleportmonst(xx, yy);

        headsup();

        update_stats();
        return (1);
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
            return (0);
        }
        break;
    }

    case SA_TAILTHWACK:
        i = rnd(25) - UU.ac;

        say("The %s hit you with its barbed tail.\n", mname);
        headsup();

        losehp(i, monst);
        return (0);

    case SA_CONFUSE:
        UU.confuse += 10 + rnd(10);
        say ("The %s has confused you.\n", mname);
        headsup();
        break;

    case SA_MULTI:  /* performs any number of other special
                     * attacks   */
        return (spattack(spsel[rund(10)], xx, yy));

    case SA_PSIONICS:
        p = "The %s flattens you with it's psionics!\n";
        i = rnd(15) + 30 - UU.ac;

        say(p, mname);
        headsup();

        losehp(i, monst);
        return (0);

    case SA_STEAL:
        if (UU.notheft) {
            return 0; /* he has device of no theft */
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
        return (1);

    case SA_BITE:
        i = rnd(10) + 5 - UU.ac;
        is_lesser_attack = true;
        /* Fall through */

    case SA_BIGBITE:
        if (!is_lesser_attack) i = rnd(15) + 10 - UU.ac;
        p = "The %s bit you!\n";

        say(p, mname);
        headsup();

        losehp(i, monst);
        return (0);
    }/* switch */

    if (p) {
        say(p, mname);
    }
    return (0);
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
            if (!verifyxy(&i, &j))  /* if not out of bounds */
                if (*(p = &Map[i][j].mon.id)) {  /* if a monster there */
                    if (*p < DEMONLORD1) {
                        k += mon_exp(*p);
                        *p = Map[i][j].know = 0;
                    } else {
                        say("The %s barely escapes being annihilated!\n",
                                MonType[*p].name);
                        /* lose half hit points */
                        Map[i][j].mon.hitp = (Map[i][j].mon.hitp >> 1) + 1;
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
    int tmp, x;

    if (lev < 1)
        lev = 1;
    if (lev > 12)
        lev = 12;

    tmp = WATERLORD;

    if (lev < 5)
        while (tmp == WATERLORD)
            tmp = rnd(((x = monstlevel[lev - 1]) != 0) ? x : 1);

    else while (tmp == WATERLORD)
        tmp = rnd(((x=monstlevel[lev-1] - monstlevel[lev-4]) != 0) ? x : 1)
            + monstlevel[lev - 4];

    while ((MonType[tmp].flags & FL_GENOCIDED) != 0 && tmp < MAXCREATURE)
        tmp++;

    if (getlevel() <= DBOTTOM)
        if (rnd(100) < 10)
            tmp = LEMMING;

    return (tmp);
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

    if (--GS.monstCount <= 0) {
        GS.monstCount = 120 - level*4;
        fillmonst(makemonst(getlevel()));
    }/* if */
}/* randmonst */


/*
 *  subroutine to put monsters into an empty room without walls or other
 *  monsters
 */
int
fillmonst (int what) {
    int x,y,trys;

    /* max # of creation attempts */
    for (trys=10; trys>0; --trys) {
        x=rnd(MAXX-2);
        y=rnd(MAXY-2);
        if ((Map[x][y].obj.type==0) && (Map[x][y].mon.id==0) &&
            ((UU.x!=x) || (UU.y!=y))) {
            Map[x][y].mon = mk_mon(what);
            Map[x][y].know=0;
            return(0);
        }
    }
    return -1; /* creation failure */
}/* fillmonst */

