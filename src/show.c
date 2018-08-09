// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "show.h"

#include "internal_assert.h"

#include "game.h"


/* Return a string describing the index'th item in inventory.  If
 * PRICEMODE is PRM_STORE or or PRM_BANK, append the price offered by
 * the Trading Post or Bank of Larn respectively.  The result is
 * static (i.e. does not need to be freed but will change between
 * calls. */
const char *
inv_line(int index, enum PRICEMODE pricemode) {
    struct Object obj = Invent[index];
    static char result[300];

    strncpy(result, knownobjname(obj), sizeof(result));

    if (iswieldable(obj) && (GS.wizardMode || obj.iarg != 0)) {
        char enbuf[30];
        snprintf(enbuf, sizeof(enbuf), " %+d", (int)Invent[index].iarg);
        strncat(result, enbuf, sizeof(result) - 1);
    }/* if */

    if (UU.wield == index)
        strncat(result, " (weapon in hand)", sizeof(result) - 1);

    if ((UU.wear == index) || (UU.shield == index))
        strncat(result, " (being worn)", sizeof(result) - 1);

    result[sizeof(result) - 1] = 0;

    if (pricemode) {
        char buffer[300];
        int price = pricemode == PRM_STORE ?
            storesellvalue(obj) : banksellvalue(obj);

        strncpy(buffer, result, sizeof(buffer));

        snprintf(result, sizeof(result), "%-40s (%d GP)", buffer, price);
    }/* if */

    return result;
}/* inv_line*/


/* Interactively display those items in inventory whose flag bits
   match a set bit in 'filter', headed by 'desc'.  Returns index in
   inventory of the selected item, -1 if nothing is selected and -2 if
   there is nothing acceptible in the inventory. */
int
inv_pick(const char *desc, unsigned filter, enum PRICEMODE pricemode) {
    int *ids = NULL;
    int count = inv_pick_multi(desc, filter, pricemode, &ids, false);

    // Return special values if there were no selections or no
    // selectables.
    if (count == 0) { return -1; }
    if (count < 0) { return -2; }

    ASSERT(count == 1);

    int result = ids[0];
    free(ids);
    return result;
}/* inv_pick*/



// Display the inventory and let the player choose zero or more
// items. Returns number of selected items and stores their IDs in
// *ids, which it allocates.  If there are no qualifying items,
// returns -2.
int
inv_pick_multi(const char *desc, unsigned filter, enum PRICEMODE pricemode,
               int **ids, bool multi) {

    struct PickList *picker = pl_malloc();

    for (int n = 0; n < IVENSIZE; n++) {
        ASSERT(IVENSIZE <= 26); /* Code below assumes <26. Fix if greater. */

        int t = Invent[n].type;
        if (t != ONONE && (filter == 0 || (Types[t].flags & filter))) {
            pl_add(picker, n, n + 'a', inv_line(n, pricemode));
        }/* if */
    }/* for */

    if (pl_count(picker) == 0) {
        pl_free(picker);
        return -2;
    }/* if */

    int count = pick_multi(picker, desc, ids, multi);
    
    pl_free(picker);

    return count;
}// inv_pick_multi





// Print a description of the inventory item at index in the message
// window.
void
show_inv_item (int index) {
    say("%c)   %s\n", 'a' + index, inv_line(index, PRM_NONE));
}/* show_inv_item */


// Interactively show the list of all known potions, scrolls and spells.
void
list_known() {
    struct PickList *picker;
    const int INDENT = 4;
       
    picker = pl_malloc();

    pl_add(picker, 0, 0, "Spells:");
    for (int n = 0; n < SPNUM; n++) {
        if (GS.spellknow[n]) {
            char buffer[80];
            snprintf(buffer, sizeof(buffer), "%*s%s", INDENT, "", Spells[n].name);
            pl_add(picker, 0, 0, buffer);
        }// if 
    }// for 

    for (int section = 0; section < 2; section++) {
        pl_add(picker, 0, 0, "");
        pl_add(picker, 0, 0, section == 0 ? "Scrolls:" : "Potions:");

        unsigned long mask = section == 0 ? OA_SCROLL : OA_POTION;
        for (int n = 0; n < OBJ_COUNT; n++) {
            if ( (Types[n].flags & mask) && Types[n].isKnown ) {
                char buffer[80];
                snprintf(buffer, sizeof(buffer), "%*s%s", INDENT, "",
                         Types[n].shortdesc);
                pl_add(picker, 0, 0, buffer);
            }// if 
        }// for
    }

    int dummy = 0;
    pick_item(picker, "Discoveries To Date:", &dummy);
    pl_free(picker);

}// list_known

