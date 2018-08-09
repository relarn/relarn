// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


/* This is the master list of monsters.  It is included in multiple
   places with different definitions for MONSTER() in order to
   construct different lists of attributes.

   Note: order is significant!  Monsters are listed in order of
   increasing difficulty.  Also, all the demons need to be
   together and LAST.
*/

/*
  Example macro definition:

  #define MONSTER(id,sym,lv,ac,dmg,attack,int,gold,hp,exp,flags,longdesc)
*/

/*      ID               SYM  LV    AC  DMG         ATTACK  INT  GOLD     HP      EXP                              FLAGS  LONG DESC */
MONSTER(NOMONST,           0,  0,    0,   0,       SA_NONE,   0,    0,     0,       0,                           FL_NONE, "")
MONSTER(LEMMING,         'l',  1,    0,   0,       SA_NONE,   3,    0,     0,       1,                           FL_NONE, "lemming")
MONSTER(GNOME,           'G',  1,   10,   1,       SA_NONE,   8,   30,     2,       2,                           FL_NONE, "gnome")
MONSTER(HOBGOBLIN,       'H',  1,   13,   2,       SA_NONE,   5,   25,     3,       2,                           FL_SLOW, "hobgoblin")
MONSTER(JACKAL,          'J',  1,    7,   1,       SA_NONE,   4,    0,     1,       1,                           FL_NONE, "jackal")
MONSTER(KOBOLD,          'K',  1,   15,   1,       SA_NONE,   7,   10,     1,       1,                           FL_NONE, "kobold")
MONSTER(ORC,             'O',  2,   15,   3,       SA_NONE,   9,   40,     5,       2,                           FL_NONE, "orc")
MONSTER(SNAKE,           'S',  2,   10,   1,       SA_NONE,   3,    0,     3,       1,                           FL_NONE, "snake")
MONSTER(CENTIPEDE,       'c',  2,   13,   1,      SA_STING,   3,    0,     2,       2,                           FL_NONE, "giant centipede")
MONSTER(JACULI,          'j',  2,    9,   1,       SA_NONE,   3,    0,     2,       1,                           FL_NONE, "jaculi")
MONSTER(TROGLODYTE,      't',  2,   10,   2,       SA_NONE,   5,   80,     5,       3,                           FL_SLOW, "troglodyte")
MONSTER(ANT,             'A',  2,    8,   1,      SA_STING,   4,    0,     5,       4,                           FL_NONE, "giant ant")
MONSTER(EYE,             'E',  3,    8,   2,       SA_NONE,   3,    0,     7,       2,                           FL_NONE, "floating eye")
MONSTER(LEPRECHAUN,      'L',  3,    3,   0,  SA_STEALGOLD,   3, 1500,    15,      40,                           FL_NONE, "leprechaun")
MONSTER(NYMPH,           'N',  3,    3,   0,      SA_STEAL,   9,    0,    20,      40,                           FL_NONE, "nymph")
MONSTER(QUASIT,          'Q',  3,    5,   3,       SA_NONE,   3,    0,    14,      10,                           FL_NONE, "quasit")
MONSTER(RUSTMONSTER,     'R',  3,    5,   0,       SA_RUST,   3,    0,    18,      20,                           FL_NONE, "rust monster")
MONSTER(ZOMBIE,          'Z',  3,   12,   3,       SA_NONE,   3,    0,     9,       7,                           FL_NONE, "zombie")
MONSTER(ASSASSINBUG,     'a',  4,    4,   3,       SA_NONE,   3,    0,    23,      13,                           FL_NONE, "assassin bug")
MONSTER(BUGBEAR,         'b',  4,    5,   4,       SA_BITE,   5,   40,    24,      33,                           FL_NONE, "bitbug")
MONSTER(HELLHOUND,       'h',  4,    5,   2,       SA_FIRE,   6,    0,    20,      33,                           FL_NONE, "hell hound")
MONSTER(ICELIZARD,       'i',  4,   11,   3, SA_TAILTHWACK,   6,   50,    19,      23,                           FL_SLOW, "ice lizard")
MONSTER(CENTAUR,         'C',  4,    6,   4,       SA_NONE,  10,   40,    25,      43,                           FL_NONE, "centaur")
MONSTER(TROLL,           'T',  5,    9,   5,       SA_NONE,   9,   80,    55,     250,                           FL_NONE, "troll")
MONSTER(YETI,            'Y',  5,    8,   4,       SA_NONE,   5,   50,    45,      90,                           FL_NONE, "yeti")
MONSTER(WHITEDRAGON,     'd',  5,    4,   5,       SA_COLD,  16,  500,    65,    1000,                           FL_NONE, "white dragon")
MONSTER(ELF,             'e',  5,    3,   3,       SA_NONE,  15,   50,    25,      33,                           FL_NONE, "elf")
MONSTER(CUBE,            'g',  5,    9,   3,       SA_NONE,   3,    0,    24,      43,                           FL_NONE, "gelatinous cube")
MONSTER(METAMORPH,       'm',  6,    9,   3,       SA_NONE,   3,    0,    32,      40,                           FL_SLOW, "metamorph")
MONSTER(VORTEX,          'v',  6,    5,   4,       SA_NONE,   3,    0,    33,      53,                           FL_NONE, "vortex")
MONSTER(ZILLER,          'z',  6,   15,   3,       SA_NONE,   3,    0,    34,      33,                           FL_NONE, "ziller")
MONSTER(VIOLETFUNGI,     'F',  6,   12,   3,       SA_NONE,   3,    0,    39,      90,                           FL_NONE, "violet fungus")
MONSTER(WRAITH,          'W',  6,    3,   1,      SA_DRAIN,   3,    0,    36,     300,                          FL_NOPIT, "wraith")
MONSTER(FORVALAKA,       'f',  6,    3,   5,       SA_NONE,   7,    0,    55,     270,                           FL_NONE, "forvalaka")
MONSTER(LAMANOBE,        'l',  7,   14,   7,       SA_NONE,   6,    0,    36,      70,                           FL_NONE, "lama nobe")
MONSTER(OSEQUIP,         'o',  7,    4,   7,    SA_BIGBITE,   4,    0,    36,      90,                           FL_NONE, "osequip")
MONSTER(ROTHE,           'r',  7,   15,   5,       SA_NONE,   3,  100,    53,     230,                           FL_NONE, "rothe")
MONSTER(XORN,            'X',  7,    6,   7,       SA_NONE,  13,    0,    63,     290,                           FL_NONE, "xorn")
MONSTER(VAMPIRE,         'V',  7,    5,   4,      SA_DRAIN,  17,    0,    55,     950,                          FL_NOPIT, "vampire")
MONSTER(INVISIBLESTALKER,' ',  7,    5,   6,       SA_NONE,   5,    0,    55,     330,                           FL_SLOW, "invisible stalker")
MONSTER(POLTERGEIST,     'p',  8,    1,   8,       SA_NONE,   3,    0,    55,     430,                          FL_NOPIT, "poltergeist")
MONSTER(DISENCHANTRESS,  'q',  8,    3,   1, SA_DISENCHANT,   3,    0,    57,     500,                           FL_NONE, "disenchantress")
MONSTER(SHAMBLINGMOUND,  's',  8,   13,   5,       SA_NONE,   6,    0,    47,     390,                           FL_NONE, "shambling mound")
MONSTER(YELLOWMOLD,      'y',  8,   12,   4,       SA_NONE,   3,    0,    37,     240,                           FL_NONE, "yellow mold")
MONSTER(UMBERHULK,       'U',  8,    6,   7,    SA_CONFUSE,  14,    0,    67,     600,                           FL_NONE, "umber hulk")
MONSTER(GNOMEKING,       'k',  9,   -1,  10,       SA_NONE,  18, 2000,   120,    3000,                           FL_NONE, "gnome king")
MONSTER(MIMIC,           'M',  9,    9,   7,       SA_NONE,   8,    0,    57,     100,                           FL_NONE, "mimic")
MONSTER(WATERLORD,       'w',  9,  -10,  15,     SA_GUSHER,  20,    0,   155,   15000,                       FL_NOBEHEAD, "water lord")
MONSTER(BRONZEDRAGON,    'D',  9,    5,   9,    SA_BIGFIRE,  16,  300,    90,    4000,                           FL_NONE, "bronze dragon")
MONSTER(GREENDRAGON,     'D',  9,    4,   4, SA_TAILTHWACK,  15,  200,    80,    2500,                           FL_NONE, "green dragon")
MONSTER(PURPLEWORM,      'P',  9,   -1,  13,       SA_NONE,   3,  100,   130,   15000,                           FL_NONE, "purple worm")
MONSTER(XVART,           'x',  9,   -2,  14,       SA_NONE,  13,    0,   100,    1000,                           FL_SLOW, "xvart")
MONSTER(SPIRITNAGA,      'n', 10,  -20,  15,      SA_MULTI,  23,    0,   100,   20000,              FL_NOBEHEAD|FL_NOPIT, "spirit naga")
MONSTER(SILVERDRAGON,    'D', 10,   -4,  10,    SA_BIGFIRE,  20,  700,   110,   10000,                          FL_NOPIT, "silver dragon")
MONSTER(PLATINUMDRAGON,  'D', 10,   -7,  15,   SA_PSIONICS,  22, 1000,   150,   25000,                          FL_NOPIT, "platinum dragon")
MONSTER(GREENURCHIN,     'u', 10,   -5,  12,       SA_NONE,   3,    0,    95,    5000,                           FL_NONE, "green urchin")
MONSTER(REDDRAGON,       'D', 10,   -4,  13,    SA_BIGFIRE,  19,  800,   120,   14000,                           FL_NONE, "red dragon")
MONSTER(DEMONLORD1,      '1', 12,  -40,  20,    SA_BIGFIRE,  20,    0,   150,   50000,     FL_DEMON|FL_NOPIT|FL_NOBEHEAD, "type I demon lord")
MONSTER(DEMONLORD2,      '2', 13,  -45,  25,       SA_COLD,  22,    0,   200,   75000,     FL_DEMON|FL_NOPIT|FL_NOBEHEAD, "type II demon lord")
MONSTER(DEMONLORD3,      '3', 14,  -50,  30, SA_DISENCHANT,  24,    0,   250,  100000,     FL_DEMON|FL_NOPIT|FL_NOBEHEAD, "type III demon lord")
MONSTER(DEMONLORD4,      '4', 15,  -55,  35,    SA_CONFUSE,  26,    0,   300,  125000,     FL_DEMON|FL_NOPIT|FL_NOBEHEAD, "type IV demon lord")
MONSTER(DEMONLORD5,      '5', 16,  -60,  40,   SA_PSIONICS,  28,    0,   350,  150000,     FL_DEMON|FL_NOPIT|FL_NOBEHEAD, "type V demon lord")
MONSTER(DEMONLORD6,      '6', 17,  -65,  45,   SA_PSIONICS,  30,    0,   400,  175000,     FL_DEMON|FL_NOPIT|FL_NOBEHEAD, "type VI demon lord")
MONSTER(DEMONLORD7,      '7', 18,  -70,  50,      SA_DRAIN,  32,    0,   450,  200000,     FL_DEMON|FL_NOPIT|FL_NOBEHEAD, "type VII demon lord")
MONSTER(DEMONPRINCE,     '9', 19, -100,  80,      SA_DRAIN,  40,    0,  1000,  500000,     FL_DEMON|FL_NOPIT|FL_NOBEHEAD, "demon prince")
MONSTER(DEMONKING,       '0', 20, -127, 127,      SA_DRAIN, 100,    0, 32767, 1000000,     FL_DEMON|FL_NOPIT|FL_NOBEHEAD, "God of Hellfire")
