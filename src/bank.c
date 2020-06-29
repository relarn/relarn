// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "bank.h"

#include "display.h"
#include "ui.h"
#include "store.h"
#include "player.h"


static void ointerest(void);
static void show_taxes_msg(const char *branchTitle);
static void bankmenu(const char *str);


/*
 *  function to put interest on your bank account
 *
 *  limit of 1 million gold pieces in bank
 */

static void
ointerest() {
    const int BANKLIMIT = 1000000;
    int i;

    if (UU.bankaccount < 0) {
        UU.bankaccount = 0;
    } else if (UU.bankaccount > 0 && UU.bankaccount < BANKLIMIT) {
        i = (UU.gtime-UU.banktime)/MOBUL; /*# mobuls elapsed since last here*/

        while ((i-- > 0) && (UU.bankaccount < BANKLIMIT)) {
            /* at 1 mobul ~=~ 1 hour, is 10 % a year */
            UU.bankaccount += (long) (UU.bankaccount / 877);
        }/* while */
    }/* if */

    UU.banktime = (UU.gtime/MOBUL)*MOBUL;
}/* ointerest*/

static void
show_taxes_msg(const char *branchTitle) {
    char amt[200];
    snprintf (amt, sizeof(amt),
             "levied taxes have been paid.  They have also told us that you "
             "owe %ld gp in",(long)UU.outstanding_taxes);

    billboard(true, branchTitle, "", "",
              "The Larn Revenue Service has ordered that your account be "
              "frozen until all",

              amt,

              "taxes and we must comply with them. We cannot serve you at "
              "this time.  Sorry.",

              "",

              "We suggest you go to the LRS office and pay your taxes.",
              NULL);
}/* show_taxes_msg*/



/*
 *  for the first national bank of Larn
 */
void
obank(bool isMain) {
    const char *title =
        isMain ? "Welcome to the First National Bank of Larn." :
        "Welcome to the 8th-level branch office of the First National "
        "Bank of Larn.";

    ointerest();    /* credit any needed interest */

    if (UU.outstanding_taxes > 0) {
        show_taxes_msg(title);
        return;
    }/* if */

    bankmenu(title);
}/* obank*/


static const char*
slogan() {
    static int slogan_count = 0;
    static const char *slogans[] = {
        "Where Service is Another S-Word.",
        "Ask us About our Sword Loans.",
        "Sure, We'll Hold Your Money for You.",
        "Trust Us.",
        "Where the Last Guy who Tried to Rob us is that Stain Over There.",
        "Your Dungeon (Business) Partner.",
        "A Long Tradition of Antitrust.",
        "Not Part of the Mortgage Crash.",
        "Your First (and Only) Choice.",
        "You're In Good Talons.",
        "Putting People Fourth (Hey, It's in the Top Five).",
        "Bank of Opportunism.",
        "Oh, Like You're Going To Lug Around Bags of Gold All Day?",
        "We Have More Than One Branch.",
        "Not A Scam.  You Have Our Word On It.",
        "We Sometimes Pay Interest.",
        "Slightly Safer Than a Hole in the Ground.",

        NULL,
    };

    /* We give the same boring slogan the first few times to lull the
     * player into a false sense of security. */
    if (UU.bankvisits < 5) {
        ++UU.bankvisits;
        return "We value your business.";
    }/* if */

    /* If unset, count the number of slogans in slogan_count. */
    for ( ; slogans[slogan_count]; slogan_count++)
        ;

    return slogans[rund(slogan_count)];
}/* slogan*/

static void
bankmenu(const char *title) {
    char action;
    char titleBuffer[512];

    snprintf(titleBuffer, sizeof(titleBuffer), "\n%s\n\n\"%s\"\n", 
             title, slogan());

    action = menu(titleBuffer,
                  "(c)heck your balance\n"
                  "(d)eposit money\n"
                  "(w)ithdraw money\n"
                  "(s)ell a gem or artifact\n"
                  "\n"
                  "(e)xit");

    switch(action) {

    case 'c':
        say("You have %d gold piece%s in the bank.\n", (long)UU.bankaccount,
            UU.bankaccount == 1 ? "" : "s");
        break;

    case 'd':
    case 'w': {
        int amt;
        char promptbuf[80];
        bool deposit = (action == 'd');
        int maxtransfer = deposit ? UU.gold : UU.bankaccount;
        
        snprintf(promptbuf, sizeof(promptbuf), "Balance: %ld GP. %s how much? ",
                 (long)UU.bankaccount, deposit ? "Deposit" : "Withdraw");
        amt = numPrompt(promptbuf, maxtransfer, maxtransfer);
        amt = clamp(amt, 0, maxtransfer);
        if (!amt) {
            say("\n");
            break;
        }/* if */

        /* Negate amt if we're withdrawing cash. */
        amt = deposit ? amt : -amt;
        
        UU.gold -= amt;
        UU.bankaccount += amt;

        say ("%s %d GP; balance is now %d GP.\n",
             deposit ? "Deposited" : "Withdrew", abs(amt), UU.bankaccount);
        break;
    }

    case 's':
        sell_multi(
            OA_BANKBUYS,
            PRM_BANK,
            "Sell which item?",
            "Unfortunately, you have nothing we are interested in.",
            "transaction cancelled.",
            "You've sold %d items for %d gp.\nThank you for choosing the Bank of Larn.\n"
            );
        break;

    default:
        break;
    }

    say("Thank you for choosing us.\n");
}/* bankmenu*/

