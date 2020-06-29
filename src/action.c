// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.



#include "internal_assert.h"
#include "display.h"
#include "show.h"
#include "game.h"
#include "look.h"
#include "ui.h"

#include "action.h"


static int whatitem(const char *str);

/*
  function to wield a weapon
*/
void
wield () {
    int i, iz;

    i = whatitem("wield");
    if (i == 0) return;

    if (i == '-') {
        say("You unwield your weapon.\n");
        UU.wield = -1;
        return;
    }/* if */

    iz = i - 'a';
    ASSERT(iz < IVENSIZE && Invent[iz].type != 0);

    /* You can't wield something you're already wielding. */
    if (UU.wield == iz) {
        say("You're already wielding that.\n");
        return;
    }/* if */

    /* You can't wield an unwieldable thing. */
    if (!iswieldable(Invent[iz])) {
        say("You can't wield item %c!\n", i); 
        return;
    }/* if */

    /* Can't wield a 2-handed sword while wearing a shield. */
    if ((UU.shield != -1) && (Invent[iz].type == O2SWORD)) {
        say("But one arm is busy with your shield!\n");
        return; 
    }/* if */

    /* Can't wield something you're currently wearing. */
    if (iz==UU.wear || iz==UU.shield) {
        say("You can't wield your %s while you're wearing it!\n",
                (iz==UU.wear) ? "armor" : "shield");
        return;
    }/* if */

    UU.wield = iz;

    say("You wield %s.\n", longobjname(Invent[iz]));
}/* wield */

/* Wear the shield at inventory index iz. */
static void
wearshield(int iz) {
    if (UU.shield != -1) { 
        say("You are already wearing %s.\n",
            iz == UU.shield ? "that" : "a shield");
        return; 
    }

    if (Invent[UU.wield].type==O2SWORD) {
        say("Your hands are busy with the two handed sword!\n");
        return; 
    }
        
    UU.shield = iz;
    if (UU.wield == iz) UU.wield = -1;
    
    say("You put on %s.\n", longobjname(Invent[iz]));
}/* wearshield*/


/*
  function to wear armor
*/
void
wear () {
    int i, iz;
    struct Object obj;

    i = whatitem("wear");
    if (i == 0) return;
    ASSERT (isalpha(i));

    iz = i - 'a';
    ASSERT (iz < IVENSIZE);

    obj = Invent[iz];
    ASSERT(iswearable(obj));

    /* Shields are a special case. */
    if (obj.type == OSHIELD) {
        wearshield(iz);
        return;
    }/* if */

    /* Otherwise, put it on! */
    if (UU.wear != -1) {
        say("You are already wearing some armor.\n");
        return; 
    }

    UU.wear=iz;
    if (UU.wield == iz) UU.wield = -1;

    say("You put on your %s.\n", longobjname(Invent[iz]));
}/* wear */


static void
askanddropgold() {
    long amt;

    amt = numPrompt("How much gold do you drop? ", UU.gold, UU.gold);
    if (amt <= 0) return;

    if (amt > UU.gold) { 
        say("You don't have that much!\n"); 
        return; 
    }/* if */

    drop_gold((unsigned long)amt);
}/* askanddropgold*/

/*
  function to drop an object
*/
void
dropobj () {
    int i;

    i = whatitem("drop");
    if (i == 0) return;

    if (i == '.') {
        askanddropgold();
        return;
    }

    ASSERT(isalpha(i));
    drop_object(i-'a');
}/* dropobj */



/*
 *  readscr()       Subroutine to read a scroll one is carrying
 */
void
readscr () {
    int i, iz;

    i = whatitem("read");
    if (i == 0)  return;
    ASSERT (isalpha(i));

    iz = i - 'a';

    if (isscroll(Invent[iz])) {
        read_scroll(Invent[iz]); 
    } else {
        ASSERT (Invent[iz].type == OBOOK);
        readbook(Invent[iz].iarg);  
    }/* if .. else*/    

    Invent[iz] = NULL_OBJ; 
}/* readscr */

/*
 *  subroutine to eat a cookie one is carrying
 */
void
eatcookie () {
    int i, iz;

    i = whatitem("eat");
    if (i == 0) return;
    ASSERT(isalpha(i));

    iz = i - 'a';
    ASSERT (Invent[iz].type==OCOOKIE);

    show_cookie();

    Invent[iz] = NULL_OBJ;;
}/* eatcookie */

/*
 *  subroutine to quaff a potion one is carrying
 */
void
quaff () {
    int i, iz;

    i = whatitem("quaff");
    if (i == 0) return;
    ASSERT (isalpha(i));

    iz = i - 'a';
    ASSERT(ispotion(Invent[iz]));

    quaffpotion(Invent[iz]); 
    Invent[iz] = NULL_OBJ;; 
}/* quaff */


/*
  Have the player select an item for the purpose outlined in 'action'.
  Returns the corresponding character or 0 if cancelled.
*/
static int
whatitem (const char *action) {
    char candidates[IVENSIZE+1];
    int n, ncan;
    unsigned long mask = 0;
    bool isWield = false, isDrop = false;
    char result;
    char *category = "";

    static struct { char *action; unsigned long mask; char *title;} masks[] = {
        {"wield",   OA_WIELDABLE, "Wieldable Items:" },
        {"quaff",   OA_POTION,    "Potions:"},
        {"wear",    OA_WEARABLE,  "Wearable Items:" },
        {"eat",     OA_EDIBLE,    "Edible Things:" },
        {"read",    OA_READABLE,  "Reading Matterials:" },
        {"drop",    0xFFFFFFFF,   "Inventory:" },
        {NULL, 0}
    };

    /* Compute the bitmask for the properties we are looking for. */
    for (n = 0; masks[n].action; n++) {
        if (strcmp(action, masks[n].action) == 0) {
            category = masks[n].title;
            mask = masks[n].mask;
            break;
        }/* if */
    }/* for */

    ASSERT (mask);

    /* Test for the operations that need special care. */
    isWield = strcmp(action, "wield") == 0;
    isDrop = strcmp(action, "drop") == 0;

    /* Construct the list of items we can act on. */
    for (n = 0, ncan = 0; n < IVENSIZE; n++) {
        enum OBJECT_ID id;

        if (Invent[n].type == 0) {
            continue;
        }/* if */

        id = Invent[n].type;
        if (id && Types[id].flags & mask) {
            candidates[ncan++] = (char)n + 'a';
        }/* if */
    }/* for */
    
    /* Make sure there's something we can use here. */
    if (ncan < 1) {
        say ("You have nothing to %s!\n", action);
        return 0;
    }/* if */

    candidates[ncan++] = 0;

    /* Display it with quickinv. */
    result = quickinv(action, candidates, isWield, isDrop);

    /* And if the user selects '*', failover to inv_pick(). */
    if (result == '?') {
        result = inv_pick(category, mask, PRM_NONE);
        if (result < 0) return 0;
        result += 'a';
    }

    return result;
}/* whatitem */

// Teleport to a location on `level`.  If level < 0, choose randomly.
// If `risky` is true, there is a chance of dying from it.
void 
teleport(bool risky, int level) {
    int new_level;

    if (risky && rnd(151) < 3) {
        game_over_probably(DDENTOMBED);  /* stuck in a rock */
    }

    /*show ?? on bottomline if been teleported  */
    //if (!UU.wizardMode) { UU.teleflag = true; }

    if (level >= 0) {
        new_level = level;
    } else if (getlevel() == 0) {
        new_level = 0;
    } else if (getlevel() <= DBOTTOM) {    /* in dungeon */
        new_level = rnd(5) + getlevel() - 3; 
        if (new_level > DBOTTOM) { new_level = DBOTTOM; }
        if (new_level<0) { new_level=0; }
    }
    else {              /* in volcano */
        new_level = rnd(4) + getlevel()-2; 
        if (new_level >= VBOTTOM) { new_level = VBOTTOM; }
        if (new_level < DBOTTOM + 1) {
            new_level=0;  /* back to surface */
        }
    }

    UU.x = rnd(MAXX-2);
    UU.y = rnd(MAXY-2);
    
    if (getlevel() != new_level) {
        setlevel(new_level, false);
    }// if 
        
    positionplayer();

    force_full_update();
    update_display();
}/* teleport*/
