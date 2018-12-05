// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
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



#define AUTOSAVE_INTERVAL 100       // TODO: make this user-configurable


struct GameState GS = { 80, false };


static bool player_action(char key);
static void play_turn(void);


/* Initialize the player using user input. */
void
makeplayer () {
    /* Set the sex and character class unless both of them have
     * already been set. */
    if (!GameSettings.sexSet || GameSettings.cclass == CCNONE) {
        if (!get_player_type(&GameSettings.cclass, &GameSettings.sex)) {
            graceful_exit("Never mind...");
        }/* if */
        GameSettings.sexSet = true;
    }/* if */

    init_new_player(GameSettings.cclass, GameSettings.sex,
                    GameSettings.difficulty);
}/* makeplayer */

void
mainloop() {
    unsigned long turn_count = 0;   // used for autosave

    onemove(DIR_CANCEL, true);  /* Let the player get in the first shot. */
    while (1) {
        onemove(DIR_CANCEL, false);

        ++turn_count;
        if (turn_count % AUTOSAVE_INTERVAL == 0) {
            enum SAVE_STATUS ss = save_game();
            if (GS.wizardMode || !ss_success(ss)) {
                say("Autosave%s (%d)\n",
                    (ss_success(ss) ? "d" : " failed!"), (int)ss);
            }// if
        }// if
    }
}/* mainloop*/


/* Flags to control certain aspects of read/do loop behaviour. */
static bool noLook = false; /* If set, disable lookforobject() this round. */

void
cancelLook() {
    noLook = true;
}/* cancelLook*/



// Perform one turn.  There are two cases for this: normal gameplay
// and when the player is running.  In the latter case, `dir` is set
// to a direction of movement and the game behaves as if the player
// had selected that direction.
//
//  Monster move is disabled if noMonMove is true.
bool
onemove(DIRECTION dir, bool noMonMove) {
    bool running = (dir != DIR_CANCEL && dir != DIR_STAY);
    bool keepRunning = !noMonMove;
    bool missedTurn = (dir == DIR_STAY);

    /* Update player stat modifications due to carried/worn stuff. */
    recalc();

    /* Deal with any object the player may step on.  If the player is
     * running, this is a good reason to stop. */
    if (!noLook && !missedTurn) {
        bool foundSomething;
        foundSomething = lookforobject();

        // Finding a thing might update some stats so we recalculate
        // here.
        if (foundSomething) {
            recalc();
        }// if

        /* If we find something while running, stop now BEFORE the
         * move.  This way, the player regains control while still on
         * the thing.*/
        if (foundSomething && running) {
            cancelLook(); /* Prevent another lookforobject() on the retry. */
            return false;
        }/* if */
    }/* if */
    noLook = false;

    /* Move the monsters. */
    if (!noMonMove) {
        if (UU.hastemonst) movemonst();
        movemonst();
    }


    /* Display and mark seen the area around the player. */
    showplayerarea();

    /* update bottom line if needed. */
    update_stats();

    /* Do the action.  If 'dir' is a legitimate direction, attempt to
     * move in that direction.  Otherwise, this is an interactive move
     * and we call play_turn() to fetch a command from the user and
     * then do what it says. */
    if (running) {
        bool success = false;
        moveplayer(dir, &success);
        keepRunning = keepRunning && success;
    } else if (!missedTurn) {
        play_turn();
    }/* if .. else*/

    /* regenerate hp and spells */
    regen();

    /* Create new monsters if appropriate. */
    randmonst();

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
        // It is safe to save the game while waiting for player input.
        enable_emergency_save();
        key = map_getch();
        disable_emergency_save();
    } while (!player_action(key));
}/* play_turn */


static void
showstr() {
    char title[80];
    snprintf(title, sizeof(title), "Inventory\n\nGold: $%d", UU.gold);
    inv_pick(title, 0, PRM_NONE);
}/* showstr*/


static bool
player_action(char key) {
    bool success = false;

    // First, various commands that will not consume a turn or alter
    // the game state. We separate these out because it's safe to
    // leave emergency saving enabled for them and some of them will
    // block on input, meaning there's a good chance we'll be there if
    // a SIGINT arrives.

    enable_emergency_save();
    switch(key) {
    case 'i':
        showstr();
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
        say("ReLarn %s\nDifficulty %ld%s\n"
            "Based on ULarn.  Free Software, NO WARRANTY!\n",
            version_str(), (long)UU.challenge,
            canDebug() ? " (Debugging Available)" : "");
        break;

    case 'Q':
        quit();
        break; /* quit */

    case 12:    /* ^R */
    case 18:    /* ^L */
        update_display(true);
        break; /*  look        */

    default:
        success = true;
    }// switch
    disable_emergency_save();

    // Return if one of the previous cases was reached.
    if (!success) { return success; }


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
        if (UU.timestop==0)
            quaff();
        break; /* quaff a potion */

    case 'd':
        if (UU.timestop==0)
            dropobj();
        break; /* to drop an object */

    case 'c':
        cast();
        break; /* cast a spell */

    case 'C':
        closedoor();
        break;

    case 'e':
        if (UU.timestop==0)
            eatcookie();
        break; /* to eat a fortune cookie */

    case 'S':
        say("Saving . . .");
        save_and_quit();
        break;

    case 'Z':
        if (UU.level>9) {
            teleport(1);
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
                switch(Map[i][j].obj.type) {
                case OTRAPDOOR:
                case ODARTRAP:
                case OTRAPARROW:
                case OTELEPORTER:
                case OELEVATORUP:
                case OELEVATORDOWN:
                    say("It's %s\n", objname(Map[i][j].obj));
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
        say("Error saving game. Not quitting.");
        return;
    }

    graceful_exit(ss == SS_SUCCESS ?
                  "Game saved."    :
                  "Game saved but backup was lost.");
}// save_and_quit



// Shut down the UI and exit with a message.
void
graceful_exit(const char *msg) {
    teardown_ui();
    if (msg) {
        printf("%s\n", msg);
    }// if

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
    score += graduated(&UU) ? score * 0.1 : 0;

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
        --UU.constitution;
        say("You feel wiiieeeeerrrrrd all over! ");
        return;
    }// if

    // Wizards only die by choice.
    if (GS.wizardMode) {
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
             "You, %s the level %ld %s, have %s.\n\n",
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


    tb_append(msg, "The Adventurer's Guild has bestowed you with the title of:\n\n");
    tb_append(msg, levelDesc(UU.level));
    tb_append(msg, "\n\n");

    snprintf(buffer, sizeof(buffer),
             "Final Score: %ld\nDifficulty: %d\n"
             "Experience Level: %ld\n",
             score, UU.challenge, UU.level);
    tb_append(msg, buffer);
    tb_append(msg, "\n\n\nThe End\n\n\n");

    tb_center_all(msg);
    showpages(msg);

    tb_free(msg);


    // Remaining output (if any) is just printed.
    teardown_ui();

    /*  Now enter the player at the end of the scoreboard.
     *
     *  We skip this if wizard is enabled (but recall that wizard !=
     *  debug mode; it is up to developers to use Wizard Mode to keep
     *  from corrupting the communal scoreboard.) */
    if ( !GS.wizardMode ) {
        bool success = newscore(score, cause == DDWINNER, getlevel(),
                                cause_desc(cause), &UU);
        if (!success) {
            printf("Error saving your score; score not saved.\n");
        }// if
    }// if

    // Send the official junk mail.
    if (cause == DDWINNER) {
        bool status = write_emails();
        if (status) {
            launch_client();
        } else {
            // Hah hah.
            printf("ERROR: The mail daemon was exorcised!\n");
        }// if .. else
    }// if

    delete_save_files();

    printf("GAME OVER\n");
    exit(0);
}/* game_over_probably*/
