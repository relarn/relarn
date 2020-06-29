// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "game.h"

#include "internal_assert.h"

#include "action.h"
#include "display.h"
#include "game.h"
#include "help.h"
#include "movem.h"
#include "score_file.h"
#include "show.h"
#include "os.h"
#include "debug.h"
#include "settings.h"
#include "bill.h"
#include "version_info.h"
#include "look.h"
#include "player.h"
#include "ui.h"
#include "savegame.h"

#define AUTOSAVE_INTERVAL 100       // TODO: make this user-configurable


static bool player_action(char key);
static void play_turn(void);


/* Initialize the player using user input. */
void
makeplayer () {
    bool nointro = GameSettings.nointro;


    /* Set the name, gender, spouse's gender and character class
     * unless all of them have already been set. */
    if (!GameSettings.nameSet ||
        !GameSettings.genderSet ||
        !GameSettings.spouseGenderSet ||
        GameSettings.cclass == CCNONE)
    {
        if (!get_player_type(GameSettings.name, sizeof(GameSettings.name),
                             &GameSettings.cclass,
                             &GameSettings.gender,
                             &GameSettings.spouseGender))
        {
            graceful_exit("Never mind...");
        }// if

        // If the player has manually created a character, we're going
        // to show them the welcome screen regardless.
        nointro = false;

        // Not really necessary but leaves the struct in a consistent
        // state.
        GameSettings.nameSet = true;
        GameSettings.genderSet = true;
        GameSettings.spouseGenderSet = true;
    }/* if */

    init_new_player(GameSettings.cclass,
                    GameSettings.gender,
                    GameSettings.spouseGender,
                    GameSettings.difficulty);

    if (!nointro) {
        welcome();   /* welcome the player to the game */
    }
}/* makeplayer */


static void
run (DIRECTION dir) {
    cancel_look(); /* So we don't stop for an object at the start. */
    while (onemove(dir))
        ;
}/* run */


void
mainloop() {
    unsigned long action_count = 0;   // used for autosave

    // Initial stat mod update
    recalc();

    while (1) {
        stash_game_state();

        onemove(DIR_CANCEL);

        ++action_count;
        if (action_count % AUTOSAVE_INTERVAL == 0) {
            enum SAVE_STATUS ss = save_game();
            if (UU.wizardMode || !ss_success(ss)) {
                say("Autosave%s%s\n",
                    (ss_success(ss) ? "d" : " failed!"),
                    ss == SS_RENAME_FAILED ? " (rename failed)" : "");
            }// if
        }// if
    }
}/* mainloop*/



// Perform one turn.  There are two cases for this: normal gameplay
// and when the player is running.  In the latter case, `dir` is set
// to a direction of movement and the game behaves as if the player
// had selected that direction.
//
// Monster move is disabled if noMonMove is true.
bool
onemove(DIRECTION dir) {
    bool running = (dir != DIR_CANCEL && dir != DIR_STAY);
    bool missedTurn = (dir == DIR_STAY);    // Paralysis, etc.

    /* Update field of view and show changes. */
    see_and_update_fov();
    update_display();

    /* Do the action.  If 'dir' is a legitimate direction, attempt to
     * move in that direction.  Otherwise, this is an interactive move
     * and we call play_turn() to fetch a command from the user and
     * then do what it says. */
    bool keepRunning = false;
    if (running) {
        moveplayer(dir, &keepRunning);
    } else if (!missedTurn) {
        play_turn();
    }/* if .. else*/

    /* regenerate hp and spells */
    regen();

    // Update player stat modifications due to carried/worn stuff.
    recalc();

    /* Create new monsters if appropriate. */
    randmonst();

    /* Deal with any object the player may step on.  If the player is
     * running, this is a good reason to stop. */
    if (!missedTurn) {
        bool foundSomething;
        foundSomething = lookforobject();

        if (foundSomething) {
            recalc();       // Recalculate in case this changes a stat
            update_display();

            /* If we find something while running, stop now BEFORE the
             * move.  This way, the player regains control while still on
             * the thing.*/
            if (running) {
                return false;
            }// if
        }// if
    }/* if */

    /* Move the monsters. */
    if (UU.hastemonst) { movemonst(); }
    movemonst();

    return keepRunning;
}/* onemove*/


/* Perform one turn of gameplay.  If 'dir' is DIR_CANCEL (the usual
 * case), a keystroke is read from the user and the relevant action is
 * performed.
 *
 * If 'dir' is a direction, the command is
 */
static void
play_turn () {
    int key;

    do {
        key = map_getch();
    } while (!player_action(key));
}/* play_turn */


static void
show_inventory() {
    char title[80];
    snprintf(title, sizeof(title), "Inventory\n\nGold: $%ld", (long)UU.gold);
    inv_pick(title, 0, PRM_NONE);
}/* show_inventory*/


static bool
player_action(char key) {
    bool success = false;

    // First, various commands that will not consume a turn or alter
    // the game state. We separate these out because it's safe to
    // leave emergency saving enabled for them and some of them will
    // block on input, meaning there's a good chance we'll be there if
    // a SIGINT arrives.

    switch(key) {
    case 'i':
        show_inventory();
        break; /* status */

    case 'D':
        list_known();
        break; /*list spells and scrolls */

    case '?':
        help();
        break; /*give the help screen*/

    case 'g':
        say("The stuff you are carrying presently weighs %d "
            "pound%s.\n",(long)packweight(), packweight()==1?"":"s");
        break;

    case 'v':
        say("ReLarn %s. Difficulty %ld%s.\n"
            "Based on ULarn. Free Software, NO WARRANTY!\n",
            version_str(), (long)UU.challenge,
            canDebug() ? " (Debugging Available)" : "");
        break;

    case 'Q':
        quit();
        break; /* quit */

    case 'o':
        showscores(true);
        break;

    case 16:    /* ^P */
        scroll_back();
        break;

    case 14:    /* ^N */
        scroll_forward();
        break;

    case 12:    /* ^R */
    case 18:    /* ^L */
        redraw();
        break;

    default:
        success = true;
    }// switch

    // If one of the previous cases was reached, update and return.
    if (!success) {
        sync_ui(false);
        return success;
    }


    // Now, we handle cases that mutate the game state.  Emergency
    // saving is (often) unsafe here.
    switch(key) {
    case 'h':   moveplayer(DIR_WEST, &success);
        break; /* west */
    case 'H':   run(DIR_WEST);
        break; /* west */
    case 'l':   moveplayer(DIR_EAST, &success);
        break; /* east */
    case 'L':   run(DIR_EAST);
        break; /* east */
    case 'j':   moveplayer(DIR_SOUTH, &success);
        break; /* south */
    case 'J':   run(DIR_SOUTH);
        break; /* south */
    case 'k':   moveplayer(DIR_NORTH, &success);
        break; /* north */
    case 'K':   run(DIR_NORTH);
        break; /* north */
    case 'u':   moveplayer(DIR_NORTHEAST, &success);
        break; /* northeast */
    case 'U':   run(DIR_NORTHEAST);
        break; /* northeast */
    case 'y':   moveplayer(DIR_NORTHWEST, &success);
        break; /* northwest */
    case 'Y':   run(DIR_NORTHWEST);
        break; /* northwest */
    case 'n':   moveplayer(DIR_SOUTHEAST, &success);
        break; /* southeast */
    case 'N':   run(DIR_SOUTHEAST);
        break; /* southeast */
    case 'b':   moveplayer(DIR_SOUTHWEST, &success);
        break; /* southwest */
    case 'B':   run(DIR_SOUTHWEST);
        break; /* southwest */

    case '.':
        break; /* do nothing, stay here */

    case ',':  /* Like '.' but looks again. */
        force_look();
        break;

    case 'w':
        wield();
        break; /* wield a weapon */

    case 'W':
        wear();
        break; /* wear armor   */

    case 'r':
        if (UU.blindCount) {
            say("You can't read anything when you're blind!\n");
        } else if (UU.timestop==0)
            readscr();
        break; /* to read a scroll */

    case 'q':
        if (UU.timestop == 0) {
            quaff();
        }// if
        break; /* quaff a potion */

    case 'd':
        if (UU.timestop==0) {
            dropobj();
        }
        break; /* to drop an object */

    case 'c':
        cast();
        break; /* cast a spell */

    case 'C':
        closedoor();
        break;

    case 'e':
        if (UU.timestop==0) {
            eatcookie();
        }
        break; /* to eat a fortune cookie */

    case 'S':
        say("Saving . . .\n");
        save_and_quit();
        break;

    case 'Z':
        if (UU.level > 9) {
            teleport(true, -1);
            break;
        }
        say("You don't know how to teleport yet.\n");
        break; /* teleport yourself    */

    case '^': {  /* identify traps */
        int count = 0;
        int i, j;

        for (j=UU.y-1; j<UU.y+2; j++) {
            if (j < 0) j=0;
            if (j >= MAXY) break;
            for (i=UU.x-1; i<UU.x+2; i++) {
                if (i < 0) i=0;
                if (i >= MAXX) break;
                switch(at(i, j)->obj.type) {
                case OTRAPDOOR:
                case ODARTRAP:
                case OTRAPARROW:
                case OTELEPORTER:
                case OELEVATORUP:
                case OELEVATORDOWN:
                    say("It's %s\n", objname(at(i, j)->obj));
                    count++;
                };
            }
        }

        if (count ==0 )
            say("No traps are visible.\n");

        break;
    }

    case '_': /* debug mode */
        debugmenu();
        break;

    case 'T':
        if (UU.shield != -1) {
            UU.shield = -1;
            say("Your shield is off.\n");
        } else {
            if (UU.wear != -1) {
                UU.wear = -1;
                say("Your armor is off.\n");
            }
            else {
                say("You aren't wearing anything.\n");
            }
        }
        break;

    case 'P':
        if (UU.outstanding_taxes>0) {
            say("You presently owe %d gp in taxes.\n", (long)UU.outstanding_taxes);
        } else {
            say("You do not owe any taxes.\n");
        }/* if .. else*/
        break;

    default:
        success = false;
    }/* switch*/

    return success;
}/* player_action*/


// Save the game and gracefully exit UNLESS there was an error.
// Assumes UI is active.
void
save_and_quit() {
    enum SAVE_STATUS ss = save_game();
    if (!ss_success(ss)) {
        say("Error saving game! Not quitting!\n");
        return;
    }

    graceful_exit(ss == SS_SUCCESS ?
                  "Game saved."    :
                  "Game saved but backup was lost.");
}// save_and_quit


volatile static bool enable_emergency_exit = true;

// Switch off emergency saving at exit.  To be used when exiting
// normally.
void
cancel_emergency_save() {
    enable_emergency_exit = false;
}

// Save on unexpected exit (including window-close on SDL)
void
emergency_save() {
    if (!enable_emergency_exit ||
        stash_op_in_progress() || !stashed_game_present()) {
        return;
    }

    save_game();
}// emergency_save


// Update stuff that wasn't saved from the current game.  Called after
// a restore.
void
post_restore_processing() {

    // There used to be a check for UU.hp <= 0 but that can't happen
    // anymore.

    // closedoor() in action.c calls cancel_look() to stop the player
    // being asked to re-open a door they just closed.  However, if
    // they save the game before moving off that square, this state is
    // lost.  We restore it here.
    if (at(UU.x, UU.y)->obj.type == OCLOSEDDOOR) {
        cancel_look();
    }/* if */
}// post_restore_processing



// Shut down the UI and exit with a message.  Note that some platforms
// won't print the message.
void
graceful_exit(const char *msg) {
    teardown_ui();
    if (msg) {
        printf("%s\n", msg);
    }// if

    cancel_emergency_save();
    exit(0);
}// graceful_exit


// Compute the score.
//
// Note that this can be called before the end of the game. While the
// result doesn't need to reflect the final score in any way, it
// SHOULD NOT modify the state of the game.
long
compute_score(bool won) {
    // Base score is cash in hand
    long score = UU.gold + UU.bankaccount;

    // Add the total value of the inventory
    for (int n = 0; n < IVENSIZE; n++) {
        long sv = storesellvalue(Invent[n]);
        if (sv > 0) {
            score += sv;
        }
    }// for

    // Bonus for experience
    score += (UU.level - 1) * 100;

    // Bonuses for winning:
    if (won) {
        score += 100000;                        // Straight bonus
        score += UU.level < 10 ? 200000 : 0;    // Winning on low exp
        score += UU.outstanding_taxes * 1000;   // No access to stores
        score += max_l(0, TIMELIMIT - UU.gtime);// Bonus for finishing early
    }// if

    // Bonus for graduating ULarn
    score += graduated() ? score * 0.1 : 0;

    // Bonus for killing the Big Bad (aka the God of Hellfire)
    if (UU.killedBigBad) {
        score += 50000;
    }// if

    // Finally, the challenge level becomes a score multiplier
    score *= 1.0 + (double)UU.challenge/5.0;

    return score;
}// compute_score


// Compute what taxes owed would be at this moment by uu.
long
compute_taxes_owed(const struct Player *uu) {
    return (uu->gold + uu->bankaccount) * TAXRATE;
}// compute_taxes_owed


// Test if 'cause' is one of the causes of death that can't be
// recovered from via life protection.
static bool
unrecoverable(int cause) {
    return
        cause == DDHELL             ||
        cause == DDWINNER           ||
        cause == DDTOOLATE          ||
        cause == DDTRAPDOORHELL     ||
        cause == DDGENIE;
}// unrecoverable


// Return the textual description of each cause of death
static const char *
cause_desc(int cause) {
    ASSERT(cause >= 0);

    if (cause < MAXCREATURE) {
        static char result[80];

        const char* mname = MonType[cause].name;
        snprintf(result, sizeof (result), "were killed by %s %s.",
                 an(mname), mname);

        return result;
    }// if

    switch(cause) {
#define ENDING(code, desc) case code: return desc;
#       include "game_endings.h"
#undef ENDING

    default:
        // This shouldn't be possible, but it suppresses a compiler
        // warning so...
        return "disappeared in a puff of logic.";
    }// switch

    return NULL;    // not reached
}// cause_desc


// End the game, displaying the player's status (winner or loser)
// along with score and related information, log the player's score
// and exit the game.
//
// If the player had protection and died of a qualifying cause,
// returns after printing a message and adjust the player's stats
// instead of exiting.
//
// 'cause' is either an entry in GAME_ENDING or a monster ID.  If it's
// a monster ID, that's the creature that killed the player.
void
game_over_probably(unsigned cause) {
    long score;

    // If life protection is active and the player isn't overly dead,
    // then we bring 'em back.
    if (UU.lifeprot > 0 && !unrecoverable(cause)) {
        --UU.lifeprot;
        UU.hp = 1;
        constitution_adjust(-1);
        say("You feel wiiieeeeerrrrrd all over!\n");
        return;
    }// if

    // Wizards only die by choice.
    if (UU.wizardMode) {
        bool really = confirm(cause == DDWINNER ?
                              "Really finish?"  :
                              "Really die?");
        if (!really) {
            UU.hp = UU.hpmax;
            return;
        }// if
    }// if

    // Let the user read the final message before continuing.  (Not
    // necessary for the winner because there's just been a big
    // presentation about the potion working.)
    if (cause != DDWINNER) {
        promptToContinue();
    }// if

    // Player's gold is the score
    score = compute_score(cause == DDWINNER);

    char buffer[200];
    struct TextBuffer *msg = tb_malloc(INF_BUFFER, 80);

    snprintf(buffer, sizeof(buffer),
             "\n\n\n"
             "You, %s the level %d %s, have %s.\n\n",
             UU.name, UU.level, ccname(UU.cclass),
             cause == DDWINNER ? "succeeded" : "died");
    tb_append(msg, buffer);

    if (cause == DDWINNER) {
        tb_append(msg,
                  "Against all odds, you managed to save your "
                  "daughter from certain death.\n\n");
    } else if (cause == DDTOOLATE) {
        tb_append(msg,
                  "In spite of your valiant efforts you arrived too late\n"
                  "and could not bear to live with that outcome.\n\n");
    } else {
        snprintf(buffer, sizeof(buffer),
                 "In spite of your best efforts, you died when you %s\n\n",
                 cause_desc(cause));
        tb_append(msg, buffer);
    }// if .. else

    if (UU.killedBigBad && (cause == DDWINNER || cause == DDTOOLATE)) {
        tb_append(msg, "You defeated the God of Hellfire in combat.\n\n");
    }// if

    tb_append(msg,
              "The Adventurer's Guild has bestowed you with the title of:\n\n");
    tb_append(msg, levelDesc(UU.level));
    tb_append(msg, "\n\n");

    snprintf(buffer, sizeof(buffer),
             "Final Score: %ld\n"
             "Experience Level: %d\n",
             score, UU.level);
    tb_append(msg, buffer);
    tb_append(msg, "\n\n\nThe End\n\n\n");

    tb_center_all(msg);
    showpages(msg);

    tb_free(msg);


    /*  Now enter the player at the end of the scoreboard and display
     *  the scores.
     *
     *  Since this is the last thing that happens before UI teardown,
     *  we wait until afterward to display an error message if there's
     *  an error here.
     *
     *  We skip this if wizard is enabled (but recall that wizard !=
     *  debug mode; it is up to developers to use Wizard Mode to keep
     *  from corrupting the communal scoreboard.) */
    bool wrote_score_file = true;
    if ( !UU.wizardMode ) {
        wrote_score_file = newscore(score, cause == DDWINNER, getlevel(),
                                    cause_desc(cause), &UU);
    }// if

    if (wrote_score_file) {
        showscores(true);
    }// if

    // Shut down the UI.
    teardown_ui();

    // And warn the user of a problem updating the score file.
    if (!wrote_score_file) {
        notify("Error saving your score; score not saved.\n");
    }// if

    // Send the official junk mail.
    if (cause == DDWINNER) {
        bool status = write_emails();
        if (status) {
            launch_client();
        } else {
            // Hah hah.
            notify("Error delivering your email: mailer daemon got exorcised."
                   "\n");
        }// if .. else
    }// if

    delete_save_files();

    printf("GAME OVER\n");

    cancel_emergency_save();
    exit(0);
}/* game_over_probably*/
