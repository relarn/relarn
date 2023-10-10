// This file is part of ReLarn; Copyright (C) 1986 - 2023; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

/* This game is bad for you. It is evil. It will rot your brain. */

#include <unistd.h>

#include "player.h"

#include "display.h"
#include "game.h"
#include "bill.h"
#include "help.h"
#include "score_file.h"
#include "ui.h"
#include "store.h"
#include "os.h"
#include "settings.h"
#include "version_info.h"


static bool only_show_scores = false;
static bool only_show_winners = false;


static void
print_help_and_exit() {
    char* cmdhelp[] = {
        "Cmd line format: relarn [-sihvb] [-o <optsfile>]",
        "  -s               show the winners scoreboard",
        "  -i               show the entire scoreboard",
        "  -h               print this help text",
        "  -v               print the version number and quit",
        "  -o <optsfile>    specify additional options file to be used",
        "  -b               enable UI debugging; may change in the future.",
        "  -f <fontpath>    path to the font to use (SDL only).",
        "  -p <size>        size of the font to use (SDL only).",
        "  -w <filename>    write scores to a file ('-' for stdout)",
        NULL
    };

    printf("%s\n", COPYRIGHT);
    printf("Version %s\n\n", version_str());

    for (int n = 0; cmdhelp[n]; n++) {
        printf("%s\n", cmdhelp[n]);
    }

    exit(0);
}// help


static void
parse_args(int argc, char *argv[]) {
    const char *optstring = "bsivhro:f:p:w:";

    while (true) {
        int i = getopt(argc, argv, optstring);
        if (i == -1) { break; }

        switch(i) {
        case 'b':
            GameSettings.drawDebugging = true;
            break;

        case 's':
            only_show_scores = true;
            only_show_winners = true;
            break;

        case 'i':
            only_show_scores = true;
            break;

        case 'w':
            if (!write_scores(optarg)) {
                notify("Error writing scores to '%s'\n", optarg);
                exit(1);
            }// if
            exit(0);
            break;

        case 'o':
            readopts(optarg);
            break;

        case 'f':
            zstrncpy(GameSettings.fontPath, optarg,
                    sizeof(GameSettings.fontPath));
            break;

        case 'p': {
            int sz = atoi(optarg);
            if (sz <= 0) {
                printf("Invalid font size: '%s'\n", optarg);
                exit(1);
            }// if
            GameSettings.fontSize = sz;
            break;
        }

        case 'h':   /* print out command line arguments */
            print_help_and_exit();
            break;

        case 'v':
            printf("%s\n", version_str());
            exit(0);
            break;

        case '?':
        default:
            printf("Invalid option; try '-h' for help.\n");
            exit(1);
        } /* end switch */
    } /* end while */
}/* parse_args*/


static void
display_scores_and_quit() {

    init_ui();

    showscores(!only_show_winners);

    teardown_ui();

    exit(0);
}// display_scores_and_quit



int
main (int argc, char *argv[]) {
    // We catch '-h' here so there's an easy way to run and exit.
    // This lets us confirm that the current executable can find all
    // of its external dependencies (shared libs/DLLs).  We bypass
    // parse_args() to skip a bunch of sanity checks that would fail
    // if this isn't part of a proper installation.
    if (argc > 1 && streq("-h", argv[1])) {
        print_help_and_exit();
        return 0;   // not reached
    }// if

    /* Initialize OS interface. */
    init_os(argv[0]);

    /* Make sure the scoreboard exists. */
    ensureboard();

    /* Load the junk mail templates. (Fatal on error.) */
    load_email_templates();

    /* Initialize the global options struct. */
    initopts();

    /* Initialize store datastructs. */
    initstore();

    // Initialize the randomizer seed
    srandom(get_random_seed());

    /* Read the settings file, then parse the argument list (in that
     * order so the command-line can override defaults). */
    readopts(cfgfile_path());
    parse_args(argc, argv);

    // Handle the '-s' or '-i' options.
    if (only_show_scores) {
        display_scores_and_quit();
    }// if

    /* restore game if present */
    enum SAVE_STATUS rs = restore_game();
    bool restorflag = ss_success(rs);
    if (!restorflag && rs != SS_NO_SAVED_GAME) {
        printf("Error loading save file: %s\nFix and try again.\n",
               rs == SS_INCOMPATIBLE_SAVE ?
               "Save file created by a different version of relarn" :
               "Save file is corrupted; backup save file may still work");
        exit(1);
    }// if

    init_ui();  /* Initialize the abstract UI. */

    /* create new game */
    if (!restorflag) {
        makeplayer();       /*  make the character that will play*/
        setlevel(0, true);  /*  make the dungeon */
    } else {
        // We need to do this after init_ui() because it can encounter
        // fixable fatal errors (e.g. missing font file) and exit.  We
        // don't want the player to lose their save file this way.
        rotate_save();
    }// if

    /* Display a welcome message.  Also displays the character class and sex. */
    say ("Welcome %sto ReLarn, %s the %s %s\n", restorflag ? "back " : "",
         UU.name, female(UU.gender), ccname(UU.cclass));

    // Inform the user that the main save file failed and this is a backup.
    if (restorflag && rs == SS_USED_BACKUP) {
        say("Save file was missing or corrupt; loaded from backup.\n");
    }// if

    force_full_update();
    update_display();   /*  show the initial dungeon */

    // Save during unexpected exits.  (Call cancel_emergency_save() to
    // disable this before a normal exit.)
    atexit(emergency_save);

    mainloop();

    return 0;
}/* main */
