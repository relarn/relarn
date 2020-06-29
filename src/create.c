// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "display.h"
#include "util.h"
#include "player.h"

#include "create.h"


/* create scroll with probability of occurrence */
struct Object newscroll() {
    static char scprob[]= {
        OSENCHANTARM, OSENCHANTARM, OSENCHANTARM, OSENCHANTARM,
        OSENCHANTWEAP, OSENCHANTWEAP, OSENCHANTWEAP, OSENCHANTWEAP, OSENCHANTWEAP,
        OSENLIGHTEN, OSENLIGHTEN, OSENLIGHTEN, OSENLIGHTEN, OSENLIGHTEN, OSENLIGHTEN,
        OSBLANK, OSBLANK, OSBLANK, OSBLANK, OSBLANK,
        OSCREATEMONST, OSCREATEMONST, OSCREATEMONST,
        OSCREATEITEM, OSCREATEITEM, OSCREATEITEM, OSCREATEITEM, OSCREATEITEM,
        OSAGGMONST, OSAGGMONST, OSAGGMONST, OSAGGMONST, OSAGGMONST,
        OSTIMEWARP, OSTIMEWARP, OSTIMEWARP, OSTIMEWARP,
        OSTELEPORT, OSTELEPORT, OSTELEPORT,
        OSAWARENESS, OSAWARENESS, OSAWARENESS, OSAWARENESS,
        OSHASTEMONST, OSHASTEMONST, OSHASTEMONST, OSHASTEMONST,
        OSMONSTHEAL, OSMONSTHEAL, OSMONSTHEAL,
        OSSPIRITPROT, OSSPIRITPROT, OSSPIRITPROT,
        OSUNDEADPROT, OSUNDEADPROT, OSUNDEADPROT, OSUNDEADPROT,
        OSSTEALTH, OSSTEALTH,
        OSMAGICMAP, OSMAGICMAP,
        OSHOLDMONST, OSHOLDMONST, OSHOLDMONST,
        OSGEMPERFECT, OSGEMPERFECT,
        OSSPELLEXT, OSSPELLEXT,
        OSIDENTIFY, OSIDENTIFY, OSIDENTIFY,
        OSREMCURSE, OSREMCURSE, OSREMCURSE, OSREMCURSE,
        OSANNIHILATE,
        OSPULVERIZE, OSPULVERIZE, OSPULVERIZE,
        OSLIFEPROT /* 81 total */
    };
    int index;

    index = rund(sizeof(scprob));
    return obj(scprob[index], 0);
}/* newscroll*/

/* return a potion created with probability of occurrence */
struct Object newpotion() {
    /* Table of potions by probability of occurrence. */
    static char potprob[] = { 
        OPSLEEP, OPSLEEP,
        OPHEALING, OPHEALING, OPHEALING,
        OPRAISELEVEL,
        OPINCABILITY, OPINCABILITY, 
        OPWISDOM, OPWISDOM,
        OPSTRENGTH, OPSTRENGTH, 
        OPCHARISMA, OPCHARISMA,
        OPDIZZINESS, OPDIZZINESS,
        OPLEARNING,
        OPGOLDDET, OPGOLDDET, OPGOLDDET,
        OPMONSTDET, OPMONSTDET, OPMONSTDET,
        OPFORGETFUL, OPFORGETFUL,
        OPWATER, OPWATER,
        OPBLINDNESS,
        OPCONFUSION,
        OPHEROISM,
        OPSTURDINESS,
        OPGIANTSTR,
        OPFIRERESIST,
        OPTREASURE, OPTREASURE,
        OPINSTHEAL, OPINSTHEAL,
        /* No Cure Dianthroritis */
        OPPOISON, OPPOISON,
        OPSEEINVIS, OPSEEINVIS
    };  /* 41 total */

    return obj(potprob[rund(sizeof(potprob))], 0);
}/* newpotion*/

/* return leather armor with random charge or NULL_OBJ if the charge is
 * zero.*/
struct Object newleather() {
    static const char nlpts[] = { 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7 };
    unsigned charge = nlpts[rund(UU.challenge ? 10 : 13)];

    if (!charge) {
        return NULL_OBJ;
    }/* if */

    return obj(OLEATHER, charge);
}/* newleather*/


/* return chain mail with random charge or NULL_OBJ if the charge is
 * zero.*/
struct Object newchain() {
    static const char nch[] = { 0, 0, 0, 1, 1, 1, 2, 2, 3, 4 };
    unsigned charge = nch[rund(11)];        

    if (!charge) {
        return NULL_OBJ;
    }/* if */

    return obj(OCHAIN, charge);
}/* newleather*/

/* return new plate armor */
struct Object newplate() {
    static const char nplt[] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 4 };
    return obj(OPLATE, nplt[rund(UU.challenge ? 3 : 11)]);
}/* newplate*/

/* return new daggers or NULL_OBJ if charge rolls to 0. */
struct Object newdagger() {
    static const char ndgg[] = { 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 4, 5 };
    unsigned charge = ndgg[rund(13)];

    if (!charge) {
        return NULL_OBJ;
    }/* if */

    return obj(ODAGGER, charge);
}/* newdagger*/

/* return + points on new swords */
struct Object newsword() {
    static const char nsw[] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 3 };
    return obj(OLONGSWORD, nsw[rund(UU.challenge ? 6 : 14)]);
}/* newsword*/

