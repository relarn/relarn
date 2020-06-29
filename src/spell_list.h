// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


/* 
   This is the master list of spells.

   It gets included in multiple places to create the various tables
   and names needed to manage the spell system.

   Spells are stored in order of increasing usefulness.  Only reorder
   them to preserve this order.

   Spell IDs begin with 'C' (for 'cast', and because 'S' was already
   taken).

   The "code" field is mostly obsolete these days, now that casting is
   done by menu. It's kept around for historical reasons and on the
   off chance chance that it's worthwhile bringing the codes back.

*/

/*  Example macro:

    #define SPELL(id, code, name, ldesc)
*/


SPELL(CPROT,          "pro", "protection",            "Generates a +2 protection field")
SPELL(CMMISSILE,      "mle", "magic missile",         "Fires a magic arrow at the target")
SPELL(CDEX,           "dex", "dexterity",             "Adds +2 to the caster's dexterity")
SPELL(CSLEEP,         "sle", "sleep",                 "Causes some monsters to go to sleep")
SPELL(CCHARM,         "chm", "charm monster",         "Some monsters may be awed at your magnificence")
SPELL(CSSPEAR,        "ssp", "sonic spear",           "The caster's hands emit a screeching sound")
SPELL(CWEB,           "web", "web",                   "Entangles enemy with strands of sticky thread")
SPELL(CSTR,           "str", "strength",              "Adds +2 to the caster's strength for a time")
SPELL(CENLIGHTEN,     "enl", "enlightenment",         "The caster becomes aware of surroundings")
SPELL(CHEALING,       "hel", "healing",               "Restores some of the caster's health")
SPELL(CCBLIND,        "cbl", "cure blindness",        "Restores sight to a blinded caster")
SPELL(CCREATEMON,     "cre", "create monster",        "Creates a monster near to the caster")
SPELL(CPHANTASM,      "pha", "phantasmal forces",     "Creates lethal illusions")
SPELL(CINV,           "inv", "invisibility",          "The caster becomes invisible")
SPELL(CFIREBALL,      "bal", "fireball",              "Creates a ball of fire that burns whatever it hits")
SPELL(CCOLD,          "cld", "cold",                  "Creates a cone of cold which freezes things")
SPELL(CPOLY,          "ply", "polymorph",             "You can find out what this does for yourself")
SPELL(CCANCEL,        "can", "cancellation",          "Disables a monster's special abilities")
SPELL(CHASTE,         "has", "haste self",            "Speeds up the caster's movements")
SPELL(CCLOUD,         "ckl", "cloud kill",            "Creates a fog of poisonous gas")
SPELL(CVPROCK,        "vpr", "vaporize rock",         "Changes rock to air")
SPELL(CDRY,           "dry", "dehydration",           "Dries up water in the immediate vicinity")
SPELL(CLIGHTNING,     "lit", "lightning",             "Causes the caster's finger to emit lightning bolts")
SPELL(CDRAIN,         "drl", "drain life",            "Subtracts hit points from both you and a monster")
SPELL(CINVULN,        "glo", "invulnerable globe",    "Summons a protective globe around you.")
SPELL(CFLOOD,         "flo", "flood",                 "Floods the immediate chamber")
SPELL(CFINGER,        "fgr", "finger of death",       "Calls on your god to back you up")
SPELL(CSCAREMON,      "sca", "scare monster",         "Terrify the monster")
SPELL(CHOLDMON,       "hld", "hold monster",          "Freezes monsters in their tracks")
SPELL(CTIMESTOP,      "stp", "time stop",             "All movement here ceases for a time")
SPELL(CTELEPORT,      "tel", "teleport away",         "Teleports a monster ...hopefully away from you")
SPELL(CMAGICFIRE,     "mfi", "magic fire",            "Creates a curtain of fire around the caster")
SPELL(CMKWALL,        "mkw", "make a wall",           "Makes a wall in the place you specify")
SPELL(CSPHERE,        "sph", "sphere of annihilation","Creates an extremely dangerous ball of chaos")
SPELL(CBANISH,        "ban", "banish",                "Removes a species from the land -- use sparingly.")
SPELL(CSUMMON,        "sum", "summon demon",          "Summons a demon to (hopefully) help you out")
SPELL(CWALLWALK,      "wtw", "walk through walls",    "Allows the player to walk through walls")
SPELL(CALTER_REALITY, "alt", "alter reality",         "God only knows what this will do")
SPELL(CPERMANENCE,    "per", "permanence",            "Makes some properties permanent (e.g. strength)")
