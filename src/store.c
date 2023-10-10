// This file is part of ReLarn; Copyright (C) 1986 - 2023; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include <stdlib.h>
#include <string.h>

#include "internal_assert.h"
#include "picklist.h"
#include "textbuffer.h"
#include "ui.h"
#include "player.h"

#include "store.h"

//
// Inventory Management
//
// This is shared by the DnD Store, Trading Post and Dealer McDope.
//

// The common inventory
struct StoreItem ShopInvent[OBJ_COUNT];
unsigned ShopInventSz = 0;


// Sort function; compare two StoreItems by price
static int
itemcmp(const void *left, const void *right) {
    return ((struct StoreItem*)left)->price - ((struct StoreItem*)right)->price;
}/* itemcmp*/


/* Append all items in Types to ShopInvent (increasing ShopInventSz as
 * needed) provided that:
 *  1) The matching entry in found[] is false.
 *  2) At least one of the item's flags match a bit in yesflag.
 *  3) No bit in noflags matches one of the item's flags.
 * Items that match are then added to found[]. */
static void
addtoshop(bool found[], unsigned yesflags, unsigned noflags) {
    int n;
    struct StoreItem Items[OBJ_CONCRETE_COUNT];
    int itemCount = 0;

    /* Find eligible objects. */
    for (n = 0; n < OBJ_CONCRETE_COUNT; n++) {
        if (found[n]) continue;
        if (!(Types[n].flags & OA_CANSELL)) {
            found[n] = true;    // We will never use these so skip them now
            continue;
        }
        if (!(Types[n].flags & yesflags)) continue;
        if (Types[n].flags & noflags) continue;

        found[n] = true;

        Items[itemCount].price = Types[n].price;
        Items[itemCount].item = obj(n, 0);
        Items[itemCount].qty = Types[n].storeQty;
        ++itemCount;
    }/* for */

    /* Sort them by price. */
    qsort(Items, itemCount, sizeof(struct StoreItem), itemcmp);

    /* Append them to ShopInvent. */
    for (n = 0; n < itemCount; n++) {
        ASSERT(ShopInventSz < OBJ_COUNT);
        ShopInvent[ShopInventSz++] = Items[n];
    }/* for */

    /* Append a blank item for sorting. */
    ShopInvent[ShopInventSz].price = 0;
    ShopInvent[ShopInventSz].item.type = 0;
    ShopInvent[ShopInventSz].qty = 0;
    ShopInventSz++;

}/* addtoshop*/


// Initialize the common inventory
void
initstore() {
    bool found[OBJ_COUNT];

    memset(found, 0, sizeof(found));

    addtoshop(found, OA_WEARABLE, 0);
    addtoshop(found, OA_WIELDABLE, OA_CHARM|OA_EDIBLE);
    addtoshop(found, OA_CHARM, 0);
    addtoshop(found, OA_POTION, 0);
    addtoshop(found, OA_SCROLL, 0);
    addtoshop(found, OA_DRUG, 0);
    addtoshop(found, 0, 0);

    ShopInventSz--;     /* Drop the final separator. */
}


// Add 'obj' to the common inventory if the store takes it.
// Otherwise, the object just disappears from the game.
static void
restock(struct Object obj) {
    int n;

    for (n = 0; n < ShopInventSz; n++) {
        if (ShopInvent[n].item.type == obj.type) {
            ++ShopInvent[n].qty;
            break;
        }/* if */
    }/* for */
}/* restock*/



//
// Common selling code (currently the DnD Store and Pad)
//

// Return Dealer McDope's own unique name for each of his wares.
const char *
dealer_name(struct Object drug) {
    switch(drug.type) {
    case OSPEED:    return "Killer Speed";
    case OACID:     return "Groovy Acid";
    case OHASH:     return "Monster Hash";
    case OSHROOMS:  return "Trippy Shrooms";
    case OCOKE:     return "Cool Coke";
    default:
        FAIL("Bad drug!");  // We shouldn't get here
        return NULL;
    }
}// dealer_name


// Create a PickList containing the relevant store's wares.  The
// argument determines which store.
static struct PickList *
make_store_inventory(bool mcdope) {
    struct PickList *inv = pl_malloc();
    int n;
    bool lastWasBlank = true;   /* True so we don't start with a blank */

    for (n = 0; n < ShopInventSz; n++) {
        char buffer[512];

        // Only McDope sells drugs
        if (isdrug(ShopInvent[n].item) != mcdope) { continue; }

        if (!lastWasBlank && ShopInvent[n].price <= 0) {
            pl_add(inv, -1, 0, "");
            lastWasBlank = true;
            continue;
        }/* if */

        if (ShopInvent[n].qty <= 0) continue;

        lastWasBlank = false;
        if (mcdope) {
            snprintf(buffer, sizeof(buffer), "%-34s %d bucks",
                     dealer_name(ShopInvent[n].item), 10*ShopInvent[n].price);
        } else {
            snprintf(buffer, sizeof(buffer), "%-40s $%d",
                     objname(ShopInvent[n].item), 10*ShopInvent[n].price);
        }
        pl_add(inv, n, 0, buffer);
    }

    return inv;
}// make_store_inventory


// Basic functionality for a store that sells stuff
static bool
store_loop(const char *heading_fmt,     // Picker heading; needs a '%d' (gold)
           const char *cant_carry,      // msg if player can't carry it
           const char *dont_got,        // msg if out of stock
           const char *no_cash,         // msg if player has no gold
           const char *you_bought_fmt,  // final result; %s (item), %d (cost)
           const char *out_of_stock,    // msg if item is out of stock
           bool isMcDope                // true if it's McDope; false DnD Store
    ) {
    int id = -1;
    const char *msg = NULL;
    struct PickList *inv = make_store_inventory(isMcDope);
    char buffer[240], heading[200];

    snprintf(heading, sizeof(heading), heading_fmt, UU.gold);

    if (pl_count(inv) == 0) {
        billboard(true, heading, "\n\n\n\n", out_of_stock, NULL);
        return false;
    }

    if (!pick_item(inv, heading, &id) || id < 0) {
        return false;
    }/* if */

    ASSERT(id < ShopInventSz);
    ASSERT(ShopInvent[id].price > 0);

    if (inv_slots_free() <= 0) {
        msg = cant_carry;
    } else if (ShopInvent[id].qty <= 0) {
        // This isn't currently reachable but we're prepared.
        // (We keep this in order to preserve the original store message.)
        msg = dont_got;
    } else if (UU.gold < ShopInvent[id].price*10) {
        msg = no_cash;
    } else {
        struct Object o = ShopInvent[id].item;
        int price = ShopInvent[id].price*10L;
        identify(o.type);

        UU.gold -= price;
        ShopInvent[id].qty--;
        take(ShopInvent[id].item, "");

        snprintf(buffer, sizeof(buffer), you_bought_fmt, longobjname(o), price);

        msg = buffer;
    }/* if */

    billboard(true, "", "", "", msg, NULL);

    pl_free(inv);

    return true;
}// store_loop






//
// Code for selling stuff in inventory
//
//


// Given a list of indexes ('forsale') of size 'count', display the
// list of items the user has selected to sell as well as the total
// price, then prompt the user for confirmation.
//
// If the user accepts, returns the amount of the sale (which may
// be 0); otherwise, returns a negative value.
static int
confirm_full_sale(int count, int forsale[], enum PRICEMODE pm) {
    const int FOOTER_MIN = 4;   // Min footer size; keep up to date w/ code
    const int ORPHAN_SZ = 4;    // Min num lines to carry to new page
    const int HEIGHT = SCREEN_H - 1;    // One line for the prompt

    struct TextBuffer *invoice = tb_malloc(INF_BUFFER, SCREEN_W - 4);

    tb_append(invoice, "You are selling the following items:\n\n");

    int total = 0;
    int lastPageTop = 0;
    for (int n = 0; n < count; n++) {
        const char *line = inv_line(forsale[n], pm);
        tb_append(invoice, "    ");
        tb_append(invoice, line);
        tb_append(invoice, "\n");

        struct Object item = Invent[forsale[n]];
        total += (pm == PRM_BANK) ? banksellvalue(item) : storesellvalue(item);

        // Prevent one or two lines being orphaned on the next page.
        //
        // If the number of lines in 'invoice' (plus the minimum
        // footer size) is within ORPHAN_SZ of the maximum, insert a
        // \f here to break to a new page.
        if (tb_num_lines(invoice) == HEIGHT - (FOOTER_MIN + ORPHAN_SZ) + 1
            && count - (n+1) >= ORPHAN_SZ)
        {
            tb_append(invoice, "\f\n");
            lastPageTop = tb_num_lines(invoice);
            tb_append(invoice, "\n\n");
        }
    }// for

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "\nOur offer is %d gp.\n", total);
    tb_append(invoice, buffer);

    // Position the prompt at the end of the line
    while (tb_num_lines(invoice) - lastPageTop < HEIGHT - 1) {
        tb_append(invoice, "\n");
    }

    tb_append(invoice, "Continue with sale?\n");

    bool confirmed = showpages_prompt(invoice, true);

    return confirmed ? total : -1;
}// confirm_full_sale


// Given a list of indexes ('forsale') into Invent of size 'count',
// remove all items from player inventory and put them back into store
// inventory.
static void
restock_all(int count, int forsale[]) {
    for (int n = 0; n < count; n++) {
        struct Object obj = inventremove(forsale[n]);
        restock(obj);
    }// for
}// restock_all


// Generic store interface for selling stuff
void
sell_multi(int filter,                      // Mask of acceptible attributes 
           enum PRICEMODE pricemode,        // store or bank pricing?
           const char *intro,               // welcome msg
           const char *nothing_to_sell,     // msg if nothing good in Invent
           const char *nosale,              // msg if sale cancelled
           const char *fmt_sold_for_price   // sold msg; %d num sold, %d total
    ) {
    ASSERT(pricemode != PRM_NONE);
    
    int *forsale = NULL;
    int nsell = inv_pick_multi(intro, filter, pricemode, &forsale, true);
    if (nsell <= 0) {
        if (nsell == -2) {
            say("%s\n", nothing_to_sell);
        }
        return;
    }

    int total = confirm_full_sale(nsell, forsale, pricemode);
    if (total < 0) {
        say("%s\n", nosale);
        free(forsale);
        return;
    }// if

    UU.gold += total;
    restock_all(nsell, forsale);
    free(forsale);

    say(fmt_sold_for_price, nsell, total);
}// sell_multi




//
// UI entry points for several stores
//
//

// Main interface for the Trading Post
void
otradepost() {
    sell_multi(
        OA_CANSELL|OA_GEM,
        PRM_STORE,
        "Welcome to the Larn Trading Post.  What would you like to sell?",
        "Please visit us when you have goods to sell.",
        "no sale.",
        "You've sold %d items for %d gp.\nThank you, come again.\n"
        );
}// otradepost

// Main interface for the DnD Store.
void
dndstore() {
    const char *headerfmt =
        "Welcome to the Larn Thrift Shoppe.\n"
        "\"Feel free to browse to your heart's content.\"\n"
        "You break 'em, you bought 'em.\n"
        "\n"
        "Your gold: $%d";

    const char *taxmsg_fmt =
        "The Larn Revenue Service has ordered us to not do business with tax evaders.\n"
        "\n"
        "They have also told us that you owe %ld gp in back taxes and, as we must\n"
        "comply with the law, we cannot serve you at this time.  So Sorry.\n";

    if (UU.outstanding_taxes>0) {
        char heading[200], tax_msg[300];

        snprintf(heading, sizeof(heading), headerfmt, UU.gold);
        snprintf(tax_msg, sizeof(tax_msg), taxmsg_fmt, UU.outstanding_taxes);

        billboard(true, heading, "\n\n", tax_msg, NULL);
        return;
    }// if

    for(;
        store_loop(headerfmt,

                   "You can't carry anything more!",

                   "Sorry, but we are out of that item.",

                   "You don't have enough gold to pay for that!",

                   "You bought %s for $%d.00.\n\n"
                   "Thank you for shopping at the Thrift Shop.\n\n"
                   "Have a nice day.",

                   "Unfortunately, we are currently out of stock on all items.",

                   false
            )
            ;
        ) { /* nothing */ }

}/* dndstore*/


// Main UI for Dealer McDope's Pad
void
opad() {

    for(;
        store_loop(
            "Hey man, welcome to Dealer McDope's Pad!"
            " I gots the some of the finest shit\n"
            "you'll find anywhere in Larn -- check it out..."
            "\n\n"
            "Looks like you got about %d bucks on you.\n\n",

            "Hey, you can't carry any more.",

            "Whoah! I musta' taken it!",

            "Whattaya trying to pull on me? You aint got the cash!",

            "Ok, here ya go, %s for %d.",

            "Sorry man, I ain't got no more of that shit.",

            true
            )
            ;
        ) { /* nothing */ }
}// opad
