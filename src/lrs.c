// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "lrs.h"

#include "ui.h"


static void
paytaxes(long amt) {
    amt = min_l(amt, UU.outstanding_taxes);
    UU.outstanding_taxes -= amt;
    UU.gold -= amt;
}// paytaxes


// Do the LRS interactions.  These are simple enough that we can do it
// in the console window using say() and numPrompt().
void
olrs() {
    say("Welcome to the Larn Revenue Service district office.\n");
    if (UU.outstanding_taxes <= 0) {
        say("We have no record of you owing us anything. "
            "Have a nice day.\n");
        return;
    }

    say("According to our records, you owe us %ld gp.\n", UU.outstanding_taxes);

    if (UU.gold == 0) {
        say("Please pay when you have the money.\n");
        return;
    }
    
    long max = UU.outstanding_taxes > UU.gold ? UU.gold : UU.outstanding_taxes;
    long payment = numPrompt("How much do you want to pay?", max, max);
    if (payment < 0) {
        say("\nVery well. Have a nice day.\n");
        return;
    }

    paytaxes(payment);
    say("Thank you for your prompt payment. Your balance is now %ld gp.\n",
        UU.outstanding_taxes);
}// olrs
