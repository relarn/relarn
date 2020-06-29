// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "diag.h"


#include "map.h"
#include "cast.h"
#include "store.h"
#include "settings.h"
#include "ui.h"


#define DIAGFILE "Diagfile"

static char *ivendef[] = {
#define OBJECT(id, sym, price, qty, rust, weight, mod, flags, desc_pre, desc) #id,
#include "object_list.h"
#undef OBJECT
    ""
};


static char *spellnames[] = {
#define SPELL(id, code, name, ldesc) #id,
#include "spell_list.h"
#undef SPELL
    ""
};



static void
showCreated(FILE *dfile) {
    int j;

    fprintf(dfile, "Created Items:\n");
    for (j = 0; j < OBJ_CONCRETE_COUNT; j++) {
        if (UU.created[j]) {
            fprintf(dfile, "    %s\n", Types[j].desc);
        }
    }
    fprintf(dfile, "\n");
}/* showCreated*/



/*
 * draw the whole screen
 */
static void
diagdrawscreen(FILE *dfile, const struct Level *lvl) {

    /* east west walls of this line */
    for (int i = 0; i < MAXY; i++) {
        for (int j = 0; j < MAXX; j++) {
            int k = lvl->map[j][i].mon.id;
            if (k) {
                fprintf(dfile, "%c", monchar(k));
            } else {
                fprintf(dfile, "%c", Types[lvl->map[j][i].obj.type].symbol);
            }
        }
        fprintf(dfile, "\n");
    }/* for */
}/* diagdrawscreen*/


static void
diagliststolen(FILE *dfile, const struct Level *lvl) {
    fprintf (dfile, "Stolen items:\n");
    for (int i = 0; i < lvl->numStolen; i++) {
        struct Object obj = lvl->stolen[i];

        fprintf (dfile, "lvl->stolen[%d].type %-12s = %d",
                 i, ivendef[obj.type], obj.type);
        fprintf (dfile, "\t%s\t+ %d\n", objname(obj), obj.iarg);
    }/* for */
}/* diagliststolen*/


// Write out the diag file into the current directory.
void
diag() {
    FILE *dfile = fopen(DIAGFILE, "w");
    if (!dfile) {
        say ("Error opening '%s' for writing.\n", DIAGFILE);
        return;
    }

    say("Diagnosing . . .\n");

    fprintf(dfile,"\n-------- Beginning of DIAG diagnostics ---------\n\n");

    fprintf(dfile, "Hit points: %2ld(%2ld)\n", (long)UU.hp, (long)UU.hpmax);

    fprintf(dfile, "gold: %ld  Experience: %ld  Character level: %ld  "
            "Level in caverns: %ld\n",
            (long) UU.gold,
            (long) UU.experience,
            (long) UU.level,
            (long) getlevel());
    fprintf(dfile, "\n\n");

    showCreated(dfile);

    fprintf(dfile, "Settings: difficulty: %d, sex: %d, name: \"%s\"\n"
            "    nointro: %d nonap: %d email: \"%s\" cclas: %d "
            "nobeep: %d\n\n",
            GameSettings.difficulty, GameSettings.gender,
            GameSettings.name, GameSettings.nointro, GameSettings.nonap,
            GameSettings.emailClient, GameSettings.cclass, GameSettings.nobeep);


    fprintf(dfile, "Inventory\n");
    for (int j = 0; j < IVENSIZE; j++) {
        fprintf (dfile, "Invent[%d].type %-12s = %d",
                 j, ivendef[Invent[j].type], Invent[j].type );
        fprintf (dfile, "\t%s", objname(Invent[j]) );
        fprintf (dfile, "\t+ %d\n", Invent[j].iarg );
    }

    fprintf(dfile, "\nDND Store:\n\n");
    for (int j = 0; j < ShopInventSz; j++) {
        struct StoreItem i = ShopInvent[j];

        fprintf (dfile, "%5d %u %s\n", i.price, (unsigned)i.qty,
                 longobjname(i.item));
    }/* for */

    fprintf(dfile, "\nHere are the maps:\n\n");

    for (int lnum = 0; lnum < NLEVELS; lnum++) {
        fprintf(dfile, "\n-------------------------------------------------"
                "------------------\n");
        fprintf(dfile, "Level %d\n", lnum);

        struct Level lvl;
        get_level_copy_at(lnum, &lvl);

        if (lvl.exists) {
            diagdrawscreen(dfile, &lvl);
            diagliststolen(dfile, &lvl);
        } else {
            fprintf(dfile, "(not created)\n");
        }// if

        fprintf(dfile, "---------------------------------------------------"
                "----------------\n");

        fflush(dfile);
    }

    fprintf(dfile, "\n\nNow for the monster data:\n\n");
    fprintf(dfile, "\nTotal types of monsters: %d\n\n", LAST_MONSTER + 1);
    fprintf(dfile, "   Monster Name      LEV  AC   DAM  ATT  GOLD   HP     EXP\n");
    fprintf(dfile, "-------------------------------------------------------"
            "----------\n"); fflush(dfile);

    for (int i = 0; i <= LAST_MONSTER; i++) {
        fprintf(dfile, "%19s  %2d  %3d ",
                MonType[i].name,
                MonType[i].level,
                MonType[i].armorclass);
        fprintf(dfile, " %3d  %3d ",
                MonType[i].damage,
                MonType[i].attack);
        fprintf(dfile, "%6d  %3d   %6ld\n",
                MonType[i].gold,
                MonType[i].hitpoints,
                (long) MonType[i].experience);
        fflush(dfile);
    }

    fprintf(dfile, "\nAvailable potions:\n\n");
    for (int i = 0; i < OBJ_CONCRETE_COUNT; i++) {
        if (Types[i].flags & OA_POTION) {
            fprintf(dfile, "%19s\n", Types[i].desc);
        }
    }/* for */
    fflush(dfile);

    fprintf(dfile, "\nAvailable scrolls:\n\n");
    for (int i = SCROLL_FIRST; i <= SCROLL_LAST; i++) {
        fprintf(dfile, "%s\n", objname(obj(i,0)));
    }
    fflush(dfile);

    fprintf(dfile, "\nSpell list:\n\n");
    fprintf(dfile, "spell#  name           description\n");
    fprintf(dfile, "-------------------------------------------------\n\n");

    for (int j = 0; j < SPNUM; j++) {
        fprintf (dfile, "%-15s %d %-10s %21s\n%s\n",
                 spellnames[j], j, Spells[j].code, Spells[j].name,
                 Spells[j].desc);
    }
    fflush(dfile);

    fprintf(dfile, "\nObject list\n\n");
    fprintf(dfile, "\nj \tObject \tName\n");
    fprintf(dfile, "---------------------------------\n");
    for (int j = 0; j < OBJ_COUNT; j++)
        fprintf(dfile, "%d \t%c \t%s\n",
                j,
                Types[j].symbol,
                Types[j].desc );
    fflush(dfile);

    fprintf(dfile,"\n-------- End of DIAG diagnostics ---------\n");
    fflush(dfile);
    fclose(dfile);

    say("Done Diagnosing.\n");
}/* diag*/
