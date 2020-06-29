// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "debug.h"

#include "ui.h"
#include "os.h"
#include "internal_assert.h"
#include "game.h"
#include "display.h"
#include "diag.h"
#include "bill.h"
#include "action.h"

#include <limits.h>


static void debugmode();
static void do_create_monster();
static void dbg_createobj();
static void dbg_allspells();


enum DBG_CMD {
    DC_MULTI,
    DC_GEARUP,
    DC_SETGOLD,
    DC_MAXLEVEL,
    DC_HIGHLEVEL,
    DC_WIZARD,
    DC_CREATEMON,
    DC_CREATEOBJ,
    DC_CREATEALL,
    DC_ALLSPELLS,
    DC_RAISESTATS,
    DC_ALLBUFFS,
    DC_MAPLEVEL,
    DC_ID,
    DC_SETTAX,
    DC_BLIND,
    DC_TELEPORT,
    DC_LEVELUP,
    DC_DIAG,
    DC_MAIL,
    DC_NOTHING,
};


static void
reveal_all() {
    set_reveal(true);
    force_full_update();
}



static void
db_makeobjs () {
    int x, y, xi, yi;
    enum OBJECT_ID obj_id;

    for (x = 0, y = 0, obj_id = 1, xi = 0, yi = 1;
         obj_id < OBJ_CONCRETE_COUNT;
         x += xi, y += yi, obj_id++) {

        at(x, y)->obj = obj(obj_id, 0);

        if (y >= MAXY - 1 && x == 0) {
            xi = 1;
            yi = 0;
        }
        else if (x >= MAXX - 1 && y >= MAXY - 1) {
            xi = 0;
            yi = -1;
        }
        else if (x >= MAXX - 1 && y == 0) {
            xi = -1;
            yi = 0;
        }/* if */

        ASSERT (!(x == 1 && y == 0));  /* Did we loop around to the start? */
    }/* for */

    reveal_all();
}/* db_makeobjs */


static void
dbg_allspells() {
    int i;
    for (i=0; i<SPNUM; i++) {
        UU.spellknow[i] = true;
    }/* for */
}/* dbg_allspells*/


static void
raise_stats() {
    raise_min(70);
}


static void
gear_up() {
    int slots = inv_slots_free();
    int i;

    if (slots < 1) Invent[0] = NULL_OBJ;
    if (slots < 2) Invent[1] = NULL_OBJ;
    if (slots < 3) Invent[2] = NULL_OBJ;

    take(obj(OPROTRING,50), "");
    take(obj(OLANCE,25), "");
    for (i=0; i<IVENSIZE; i++) {
        if (Invent[i].type==OLANCE && Invent[i].iarg==25) {
            UU.wield=i;
            break;
        }/* if */
    }/* for */

    UU.wear = UU.shield = -1;
    take(obj(OORB, 0), "");
}/* gear_up*/


static void
debugmode() {
    UU.wizardMode = true;

    raise_stats();
    gear_up();
    raiseexperience(370*1000000);

    /* Give player all spells and all object knowledge. */
    dbg_allspells();
    reveal_all();
    identify_all();

    db_makeobjs();

    UU.gold+=250000;
}/* debugmode*/


static void
dbg_createobj() {
    struct PickList *pl;
    int n, id, iarg;
    bool status;

    pl = pl_malloc();

    for (n = 1; n < OBJ_CONCRETE_COUNT; n++) {
        char buffer[100];
        snprintf (buffer, sizeof(buffer), "'%c' %s", Types[n].symbol,
                  Types[n].desc);

        pl_add(pl, n, 0, buffer);
    }

    status = pick_item(pl, "Create which object?", &id);
    pl_free(pl);
    if (!status) return;

    iarg = numPromptAll("IArg value: ", 0, 0, 0, &status);
    if (!status) return;

    createitem(UU.x,UU.y, obj(id, iarg));
}/* dbg_createobj*/



static void
do_create_monster() {
    static struct {
        const char *monster;
        const char sym;
        enum MONSTER_ID id;
    } creatures[] = {
#define MONSTER(id,sym,lv,ac,dmg,attack,int,gold,hp,exp,flags,longdesc) \
        {longdesc, sym, id},
#       include "monster_list.h"
#undef MONSTER
        {NULL, 0, 0}
    };
    char name[80];
    enum MONSTER_ID id = 0;
    int n;

    if (!stringPrompt("Monster map symbol? ", name, sizeof(name))) {
        say("Cancelled!\n");
        return;
    }

    for (n = 0; creatures[n].monster; n++) {
        if (creatures[n].sym == *name) {
            id = creatures[n].id;
            break;
        }/* if */
    }/* for */

    if (!id) {
        say ("Unknown monster: %s\n", name);
        return;
    }/* if */

    say("createmonster(%d) (%c, %s)\n", id, *name, MonType[id].name);
    createmonster(id);
}/* do_create_monster*/


static void
set_gold() {
    long gold;
    long defaultGold = UU.gold == 0 ? 250000 : 0;

    gold = numPrompt("Set gold ", defaultGold, LONG_MAX);
    if (gold < 0) return;

    UU.gold = gold;
}/* set_gold*/


static void
set_tax() {
    long tax;

    tax = numPrompt("Set tax ", 0, LONG_MAX);
    if (tax < 0) return;

    UU.outstanding_taxes = tax;
}/* set_tax*/


static void
toggle_blindness() {
    const int BLIND_COUNT = 100;

    if (UU.blindCount) {
        UU.blindCount = 0;
        say("Blindness removed.\n");
    } else {
        UU.blindCount += BLIND_COUNT;
        say("Added %d turns of blindness.\n", BLIND_COUNT);
    }/* if .. else*/
}/* toggle_blindness*/


static void
debug_teleport() {
    long level;

    level = numPrompt("Which level do you wish to teleport to? ", 0, 20);
    if (level > 20 || level < 0) {
        say(" Cancelled!\n");
        return;
    }

    say("\n");

    teleport(false, level);
}// debug_teleport

static void
dbg_allbuffs() {
    UU.stealth          += 200;
    UU.undeadpro        += 200;
    UU.spiritpro        += 200;
    UU.charmcount       += 200;
    UU.timestop         += 20;      // Complicates stuff so it's short
    UU.holdmonst        += 200;
    UU.giantstr         += 200;
    UU.fireresistance   += 200;
    UU.dexCount         += 200;
    UU.strcount         += 200;
    UU.scaremonst       += 200;
    UU.hasteSelf        += 200;
    UU.cancellation     += 200;
    UU.invisibility     += 200;
    UU.altpro           += 200;
    UU.protectionTime   += 200;
    UU.wtw              += 200;
}// dbg_allbuffs

static enum DBG_CMD
dbg_select() {
    struct PickList *picker;
    int n, result;
    static struct {
        enum DBG_CMD cmd;
        char *desc;
    } labels[] = {
        {DC_MULTI,      "Enable most."},
        {DC_GEARUP,     "Fast equip lance, ring and awareness."},
        {DC_SETGOLD,    "Set gold."},
        {DC_MAXLEVEL,   "Set experience level to 100."},
        {DC_HIGHLEVEL,  "Set experience level to 60 (or so)."},
        {DC_WIZARD,     "Toggle wizard mode."},
        {DC_CREATEMON,  "Create a monster."},
        {DC_CREATEOBJ,  "Create an object."},
        {DC_CREATEALL,  "Create one of each object."},
        {DC_ALLSPELLS,  "Learn all spells."},
        {DC_ALLBUFFS,   "Enable all buffs."},
        {DC_RAISESTATS, "Raise all stats to 70."},
        {DC_MAPLEVEL,   "Reveal the current level."},
        {DC_ID,         "Identify all items."},
        {DC_SETTAX,     "Set taxes owed."},
        {DC_BLIND,      "Toggle blindness."},
        {DC_TELEPORT,   "Teleport to a different level."},
        {DC_LEVELUP,    "Gain one level."},
        {DC_DIAG,       "Write out a diag file."},
        {DC_MAIL,       "Create and read the junk mail."},
        {DC_NOTHING,    "Do nothing."},
        {0, NULL},
    };
    const char *desc = "Debug mode menu:";

    picker = pl_malloc();

    for (n = 0; labels[n].desc; n++) {
        pl_add(picker, labels[n].cmd, 'a'+n, labels[n].desc);
    }

    if (!pick_item(picker, desc, &result)) return DC_NOTHING;
    return result;
}/* dbg_select*/


void debugmenu() {
    if (!canDebug()) return;

    switch(dbg_select()) {
    case DC_MULTI:
        debugmode();
        break;

    case DC_GEARUP:
        gear_up();
        break;

    case DC_SETGOLD:
        set_gold();
        return;

    case DC_MAXLEVEL:
        raiseexperience(370 * 1000000);
        break;

    case DC_HIGHLEVEL:
        raiseexperience(90 * 1000000);
        break;

    case DC_WIZARD:
        UU.wizardMode = !UU.wizardMode;
        say("%s wizard mode.\n", UU.wizardMode ? "Enabled" : "Disabled");
        break;

    case DC_CREATEMON:
        do_create_monster();
        break;

    case DC_CREATEOBJ:
        dbg_createobj();
        break;

    case DC_ALLSPELLS:
        dbg_allspells();
        break;

    case DC_ALLBUFFS:
        dbg_allbuffs();
        break;

    case DC_RAISESTATS:
        raise_stats();
        break;

    case DC_MAPLEVEL:
        reveal_all();
        break;

    case DC_CREATEALL:
        db_makeobjs();
        break;

    case DC_ID:
        identify_all();
        break;

    case DC_SETTAX:
        set_tax();
        break;

    case DC_BLIND:
        toggle_blindness();
        break;

    case DC_TELEPORT:
        debug_teleport();
        break;

    case DC_LEVELUP:
        raiselevel();
        raisemhp(1);
        break;

    case DC_DIAG:
        diag();
        break;

    case DC_MAIL:
        if ( !write_emails() ) { say("Error creating junk mail.\n"); }
        break;

    default:
        return;
    }/* switch*/

}/* debugmenu*/
