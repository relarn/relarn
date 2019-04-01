// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

/*
  This is the master list of objects.  It is included in multiple
  places with different definitions for OBJECT() in order to
  construct different lists of attributes.

  Note: 
        -prices are in tens of GP. 
        -rust is positive; if non-zero, its negated value is the lowest
         something can be rusted down to.
        -weight is the pack weight and only makes sense for things that can
         be picked up.
        -MOD is the score modifier the object provides (e.g. AC for
         armor, WC for weapons)
        -DESCRIPTION is split into two parts. Most uses should just combine
         the two but the second part is useful for things with a common type
         (e.g. "This potion looks like it might give you %s.")
*/

/* Example macro:

   #define OBJECT(id, sym, price, qty, rust, weight, mod, flags, desc_pre, desc)
*/

/*     ID              MAP-SYM PRICE QTY RUST WGHT  MOD  OBJECT ATTRIBUTES                               DESCRIPTION */
OBJECT(ONONE,            ' ',     0,  0,   0,  0,    0,  OA_NONE,                                        "", "nothing")
OBJECT(OALTAR,           'A',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "holy altar")
OBJECT(OTHRONE,          'T',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "handsome, jewel-encrusted throne")
OBJECT(OORB,             'o', 10000,  0,   0,  4,    0,  OA_MOVABLE|OA_CANSELL,                          "an ", "orb of enlightenment")
OBJECT(OPIT,             'P',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "pit")
OBJECT(OSTAIRSUP,        '%',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "staircase leading upwards")
OBJECT(OELEVATORUP,      '^',     0,  0,   0,  0,    0,  OA_NONE,                                        "an ", "elevator going up")
OBJECT(OFOUNTAIN,        'F',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "bubbling fountain")
OBJECT(OSTATUE,          '&',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "great marble statue")
OBJECT(OTELEPORTER,      '^',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "teleport trap")
OBJECT(OSCHOOL,          '+',     0,  0,   0,  0,    0,  OA_NONE,                                        "the ", "University of Larn")
OBJECT(OMIRROR,          'M',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "mirror")
OBJECT(ODNDSTORE,        '=',     0,  0,   0,  0,    0,  OA_NONE,                                        "the ", "DND store")
OBJECT(OSTAIRSDOWN,      '%',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "staircase going down")
OBJECT(OELEVATORDOWN,    '^',     0,  0,   0,  0,    0,  OA_NONE,                                        "an ", "elevator going down")
OBJECT(OBANK,            '$',     0,  0,   0,  0,    0,  OA_NONE,                                        "the ", "Bank of Larn")
OBJECT(OBANK2,           '$',     0,  0,   0,  0,    0,  OA_NONE,                                        "the ", "8th branch of the Bank of Larn")
OBJECT(ODEADFOUNTAIN,    'f',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "dead fountain")
OBJECT(OGOLDPILE,        '*',     0,  0,   0,  0,    0,  OA_NONE,                                        "", "gold")
OBJECT(OOPENDOOR,        'O',     0,  0,   0,  0,    0,  OA_NONE,                                        "an ", "open door")
OBJECT(OCLOSEDDOOR,      'D',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "closed door")
OBJECT(OWALL,            '#',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "wall")
OBJECT(OTRAPARROW,       '^',     0,  0,   0,  0,    0,  OA_NONE,                                        "an ", "arrow trap")
OBJECT(OTRAPARROWIV,     ' ',     0,  0,   0,  0,    0,  OA_NONE,                                        "an ", "arrow trap")
OBJECT(OLARNEYE,         '~',     0,  0,   0,  1,    0,  OA_MOVABLE|OA_BANKBUYS,                         "The ", "Eye of Larn")
OBJECT(OPLATE,           ']',  4000,  1,   8, 35,    9,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "", "plate mail")
OBJECT(OCHAIN,           '[',   850,  2,   5, 23,    6,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "", "chain mail")
OBJECT(OLEATHER,         '[',    20,  3,   0,  8,    2,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "", "leather armor")
OBJECT(ORING,            '[',   400,  2,   4, 20,    5,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "", "ring mail")
OBJECT(OSTUDLEATHER,     '[',   100,  2,   2, 15,    3,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "", "studded leather armor")
OBJECT(OSPLINT,          ']',  2200,  1,   6, 26,    7,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "", "splint mail")
OBJECT(OPLATEARMOR,      ']',  9000,  1,   9, 40,   10,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "", "plate armor")
OBJECT(OSSPLATE,         ']', 26000,  1,   0, 40,   12,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "", "stainless plate armor")
OBJECT(OSHIELD,          '[',  1500,  1,   1,  7,    2,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "a ", "shield")
OBJECT(OELVENCHAIN,      ']', 50000,  0,   0, 15,   15,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL|OA_WEARABLE, "", "elven chain")
OBJECT(OSWORDofSLASHING, ')', 20000,  0,   0, 15,   30,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "a ", "sword of slashing")
OBJECT(OHAMMER,          ')', 75000,  0,   0, 30,   35,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "", "Bessman's flailing hammer")
OBJECT(OSWORD,           ')', 50000,  1,   0, 20,   32,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "a ", "sunsword")
OBJECT(O2SWORD,          '(', 10000,  2,   0, 23,   26,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "a ", "two-handed sword")
OBJECT(OSPEAR,           '(',   200,  3,   0,  8,   10,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "a ", "spear")
OBJECT(ODAGGER,          '(',    20,  3,   0,  1,    3,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "a ", "dagger")
OBJECT(OBATTLEAXE,       ')',  1500,  2,   0, 23,   17,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "a ", "battle axe")
OBJECT(OLONGSWORD,       ')',  4500,  2,   0, 20,   22,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "a ", "longsword")
OBJECT(OFLAIL,           '(',   800,  2,   0, 20,   14,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "a ", "flail")
OBJECT(OLANCE,           '(',200000,  1,   0, 15,   20,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "a ", "lance of death")
OBJECT(OVORPAL,          ')', 80000,  0,   0,  1,   22,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "the ", "Vorpal Blade")
OBJECT(OSLAYER,          ')',100000,  0,   0, 15,   30,  OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,             "", "Slayer")
OBJECT(ORINGOFEXTRA,     '|', 10000,  1,   0,  1,    0,  OA_CHARM|OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,    "a ", "ring of extra regeneration")
OBJECT(OREGENRING,       '|',  2200,  1,   0,  1,    1,  OA_CHARM|OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,    "a ", "ring of regeneration")
OBJECT(OPROTRING,        '|',  1500,  1,   0,  1,    1,  OA_CHARM|OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,    "a ", "ring of protection")
OBJECT(OENERGYRING,      '|',  1800,  1,   0,  1,    1,  OA_CHARM|OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,    "an ", "energy ring")
OBJECT(ODEXRING,         '|',  1200,  1,   0,  1,    0,  OA_CHARM|OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,    "a ", "ring of dexterity")
OBJECT(OSTRRING,         '|',   850,  1,   0,  1,    0,  OA_CHARM|OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,    "a ", "ring of strength")
OBJECT(OCLEVERRING,      '|',  1200,  1,   0,  1,    0,  OA_CHARM|OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,    "a ", "ring of cleverness")
OBJECT(ODAMRING,         '|',  1250,  1,   0,  1,    1,  OA_CHARM|OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,    "a ", "ring of increase damage")
OBJECT(OBELT,            '{',  2800,  1,   0,  4,    2,  OA_CHARM|OA_WIELDABLE|OA_MOVABLE|OA_CANSELL,    "a ", "belt of striking")


/* Scrolls.  SCROLL_FIRST and SCROLL_LAST in object.h *must* refer to
 * the first and last scroll in this sequence. */
OBJECT(OSENCHANTARM,     '?',  1000,  2,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "enchant armor")
OBJECT(OSENCHANTWEAP,    '?',  1250,  2,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "enchant weapon")
OBJECT(OSENLIGHTEN,      '?',   600,  4,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "enlightenment")
OBJECT(OSBLANK,          '?',   100,  4,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "blank paper")
OBJECT(OSCREATEMONST,    '?',  1000,  3,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "create monster")
OBJECT(OSCREATEITEM,     '?',  2000,  2,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "create artifact")
OBJECT(OSAGGMONST,       '?',  1100,  1,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "aggravate monsters")
OBJECT(OSTIMEWARP,       '?',  5000,  2,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "time warp")
OBJECT(OSTELEPORT,       '?',  2000,  2,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "teleportation")
OBJECT(OSAWARENESS,      '?',  2500,  4,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "expanded awareness")
OBJECT(OSHASTEMONST,     '?',   200,  5,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "haste monsters")
OBJECT(OSMONSTHEAL,      '?',   300,  3,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "monster healing")
OBJECT(OSSPIRITPROT,     '?',  3400,  1,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "spirit protection")
OBJECT(OSUNDEADPROT,     '?',  3400,  1,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "undead protection")
OBJECT(OSSTEALTH,        '?',  3000,  2,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "stealth")
OBJECT(OSMAGICMAP,       '?',  4000,  2,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "magic mapping")
OBJECT(OSHOLDMONST,      '?',  5000,  2,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "hold monsters")
OBJECT(OSGEMPERFECT,     '?', 10000,  1,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "gem perfection")
OBJECT(OSSPELLEXT,       '?',  5000,  1,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "spell extension")
OBJECT(OSIDENTIFY,       '?',  3400,  2,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "identify")
OBJECT(OSREMCURSE,       '?',  2200,  3,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "remove curse")
OBJECT(OSANNIHILATE,     '?', 39000,  0,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "annihilation")
OBJECT(OSPULVERIZE,      '?',  6100,  1,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "pulverization")
OBJECT(OSLIFEPROT,       '?', 30000,  0,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE|OA_SCROLL,    "a scroll of ", "life protection")


/* Potions.  Do not reorder!  Note that POTION_FIRST and POTION_LAST
 * in object.h must point to the first and last potion here. */
OBJECT(OPSLEEP,          '!',   200,  6,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "sleep")
OBJECT(OPHEALING,        '!',   900,  5,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "healing")
OBJECT(OPRAISELEVEL,     '!',  5200,  1,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "raise level")
OBJECT(OPINCABILITY,     '!',  1000,  2,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "increase ability")
OBJECT(OPWISDOM,         '!',   500,  2,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "wisdom")
OBJECT(OPSTRENGTH,       '!',  1500,  2,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "strength")
OBJECT(OPCHARISMA,       '!',   700,  1,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "raise charisma")
OBJECT(OPDIZZINESS,      '!',   300,  7,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "dizziness")
OBJECT(OPLEARNING,       '!',  2000,  1,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "learning")
OBJECT(OPGOLDDET,        '!',   500,  1,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "gold detection")
OBJECT(OPMONSTDET,       '!',   800,  1,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "monster detection")
OBJECT(OPFORGETFUL,      '!',   300,  3,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "forgetfulness")
OBJECT(OPWATER,          '!',   200,  5,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "water")
OBJECT(OPBLINDNESS,      '!',   400,  3,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "blindness")
OBJECT(OPCONFUSION,      '!',   350,  2,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "confusion")
OBJECT(OPHEROISM,        '!',  5200,  1,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "heroism")
OBJECT(OPSTURDINESS,     '!',   900,  2,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "sturdiness")
OBJECT(OPGIANTSTR,       '!',  2000,  2,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "giant strength")
OBJECT(OPFIRERESIST,     '!',  2200,  4,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "fire resistance")
OBJECT(OPTREASURE,       '!',   800,  6,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "treasure finding")
OBJECT(OPINSTHEAL,       '!',  3700,  3,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "instant healing")
OBJECT(OPCUREDIANTH,     '!',     0,  0,   0,  1,    0,  OA_MOVABLE|OA_POTION,                           "a magic potion of ", "cure dianthroritis")
OBJECT(OPPOISON,         '!',   500,  1,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "poison")
OBJECT(OPSEEINVIS,       '!',  1500,  3,   0,  1,    0,  OA_MOVABLE|OA_POTION|OA_CANSELL,                "a magic potion of ", "see invisible")

OBJECT(OBOOK,            'B',  2000,  1,   0,  1,    0,  OA_MOVABLE|OA_CANSELL|OA_READABLE,              "a ", "book")
OBJECT(OCHEST,           'C',  5900,  1,   0, 30,    0,  OA_MOVABLE|OA_CANSELL,                          "a ", "chest")
OBJECT(OAMULET,          '.',  4000,  1,   0,  1,    0,  OA_CHARM|OA_MOVABLE|OA_CANSELL,                 "an ", "amulet of invisibility")
OBJECT(OORBOFDRAGON,     'o',  8500,  0,   0,  4,    0,  OA_CHARM|OA_MOVABLE|OA_CANSELL,                 "an ", "orb of dragon slaying")
OBJECT(OSPIRITSCARAB,    '.',  7500,  0,   0,  1,    0,  OA_CHARM|OA_MOVABLE|OA_CANSELL,                 "a ", "scarab of negate spirit")
OBJECT(OCUBE_of_UNDEAD,  '.',  5000,  0,   0,  1,    0,  OA_CHARM|OA_MOVABLE|OA_CANSELL,                 "a ", "cube of undead control")
OBJECT(ONOTHEFT,         '.',  6000,  0,   0,  1,    0,  OA_CHARM|OA_MOVABLE|OA_CANSELL,                 "a ", "device of theft-prevention")
OBJECT(ODIAMOND,         '<',     0,  0,   0,  1,    0,  OA_MOVABLE|OA_GEM|OA_BANKBUYS,                  "a ", "brilliant diamond")
OBJECT(ORUBY,            '<',     0,  0,   0,  1,    0,  OA_MOVABLE|OA_GEM|OA_BANKBUYS,                  "a ", "ruby")
OBJECT(OEMERALD,         '<',     0,  0,   0,  1,    0,  OA_MOVABLE|OA_GEM|OA_BANKBUYS,                  "an ", "enchanting emerald")
OBJECT(OSAPPHIRE,        '<',     0,  0,   0,  1,    0,  OA_MOVABLE|OA_GEM|OA_BANKBUYS,                  "a ", "sparkling sapphire")
OBJECT(OENTRANCE,        'E',     0,  0,   0,  0,    0,  OA_NONE,                                        "the ", "dungeon entrance")
OBJECT(OEXIT,            'X',     0,  0,   0,  0,    0,  OA_NONE,                                        "the ", "dungeon exit")
OBJECT(OVOLDOWN,         'V',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "volcanic shaft leaning downward")
OBJECT(OVOLUP,           'V',     0,  0,   0,  0,    0,  OA_NONE,                                        "the ", "base of a volcanic shaft")
OBJECT(OHOME,            'H',     0,  0,   0,  0,    0,  OA_NONE,                                        "your ", "home")
OBJECT(OIVDARTRAP,       ' ',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "dart trap")
OBJECT(ODARTRAP,         '^',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "dart trap")
OBJECT(OTRAPDOOR,        '^',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "trapdoor")
OBJECT(OIVTRAPDOOR,      ' ',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "trapdoor")
OBJECT(OTRADEPOST,       'S',     0,  0,   0,  0,    0,  OA_NONE,                                        "the ", "Larn trading post")
OBJECT(OIVTELETRAP,      ' ',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "teleport trap")
OBJECT(ODEADTHRONE,      't',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "massive throne")
OBJECT(OANNIHILATION,    's',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "sphere of annihilation")
OBJECT(OTHRONE2,         'T',     0,  0,   0,  0,    0,  OA_NONE,                                        "a ", "handsome, jewel-encrusted throne")
OBJECT(OLRS,             'L',     0,  0,   0,  0,    0,  OA_NONE,                                        "the ", "Larn Revenue Service")
OBJECT(OCOOKIE,          'c',   100,  3,   0,  1,    0,  OA_MOVABLE|OA_EDIBLE|OA_CANSELL,                "a ", "fortune cookie")
OBJECT(OBRASSLAMP,       '.',   500,  0,   0,  1,    0,  OA_MOVABLE|OA_CANSELL,                          "a ", "brass lamp")
OBJECT(OHANDofFEAR,      '.',  6660,  0,   0,  1,    0,  OA_CHARM|OA_MOVABLE|OA_CANSELL,                 "The ", "Hand of Fear")
OBJECT(OSPHTALISMAN,     '.',  3000,  0,   0,  1,    0,  OA_CHARM|OA_MOVABLE|OA_CANSELL,                 "The ", "Talisman of the Sphere")
OBJECT(OWWAND,           '/',  1500,  0,   0,  1,    0,  OA_CHARM|OA_MOVABLE|OA_CANSELL,                 "a ", "wand of wonder")
OBJECT(OPSTAFF,          '/', 95000,  0,   0, 20,    0,  OA_CHARM|OA_MOVABLE|OA_CANSELL,                 "a ", "staff of power")
OBJECT(ODIPLOMA,         '\'',    0,  0,   0,  1,    0,  OA_CHARM|OA_MOVABLE,                            "your ", "diploma (BH from ULarn)")
OBJECT(OSPEED,           ':',    10,  1,   0,  1,    0,  OA_MOVABLE|OA_DRUG|OA_CANSELL,                  "some ", "speed")
OBJECT(OACID,            ':',    25,  1,   0,  1,    0,  OA_MOVABLE|OA_DRUG|OA_CANSELL,                  "some ", "LSD")
OBJECT(OHASH,            ':',    50,  1,   0,  1,    0,  OA_MOVABLE|OA_DRUG|OA_CANSELL,                  "some ", "hashish")
OBJECT(OSHROOMS,         ':',   100,  1,   0,  1,    0,  OA_MOVABLE|OA_DRUG|OA_CANSELL,                  "some ", "magic mushrooms")
OBJECT(OCOKE,            ':',   500,  1,   0,  1,    0,  OA_MOVABLE|OA_DRUG|OA_CANSELL,                  "some ", "cocaine")
OBJECT(OPAD,             '@',     0,  0,   0,  0,    0,  OA_NONE,                                        "", "Dealer McDope's Pad")

// Value to store in MapSquare.recalled to indicate that this cell has
// not been visited.  It should NEVER appear anywhere else in the
// game.  Note that code assumes that this is the first non-object
// entry.
OBJECT(OUNSEEN,          ' ',     0,  0,   0,  0,    0,  OA_NONE,                                        "an", "unexplored location")
