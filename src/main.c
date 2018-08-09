// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
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


static char cmdhelp[] = 
    "Cmd line format: relarn [-sicnh] [-o <optsfile>] [-d #] [-r]\n"
    "  -s   show the winners scoreboard\n"
    "  -i   show the entire scoreboard\n"
    "  -n   suppress welcome message on starting game\n"
    "  -h   print this help text\n"
    "  -v   print the version number and quit\n"
    "  -o <optsfile> specify additional options file to be used\n"
    "  -d # specify level of difficulty (example: relarn -d 5)\n";
    


static void
parse_args(int argc, char *argv[]) {
    const char *optstring = "sinvhro:d:";
    int i;

    while ((i = getopt(argc, argv, optstring)) != -1) {
        switch(i) {
        case 's':
            showscores(false);
            exit(0);  /* show scoreboard   */
        case 'i':
            showscores(true);
            exit(0);  /* show all scoreboard */
        case 'n':
            GameSettings.nointro = true;
            break;

        case 'o':
            readopts(optarg);
            break;

        case 'd':   /* specify hardness */
            GameSettings.difficulty = atoi(optarg);
            if (GameSettings.difficulty > 100)
                GameSettings.difficulty = 100;
            if (GameSettings.difficulty < 0) {
                printf("difficulty level must be > 0\n");
                puts(cmdhelp);
                exit(1);
            }
            break;

        case 'h':   /* print out command line arguments */
            printf("%s\n", COPYRIGHT);
            printf("Version %s\n", version_str());
            printf("\n%s", cmdhelp);
            exit(0);
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



int
main (int argc, char *argv[]) {

    /* Initialize OS interface. */
    init_os(argc, argv);

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
        makeplayer();   /*  make the character that will play*/
        setlevel(0);/*  make the dungeon */

        /* tell signals that we are in the welcome screen */
        if (!GameSettings.nointro)
            welcome();   /* welcome the player to the game */
    }

    update_display(true);   /*  show the initial dungeon */

    /* Display a welcome message.  Also displays the character class and sex. */
    say ("Welcome %sto xlarn, %s the %s %s\n", restorflag ? "back " : "",
         UU.name, UU.sex == MALE ? "male" : "female", ccname(UU.cclass));

    // Inform the user that the main save file failed and this is a backup.
    if (restorflag && rs == SS_USED_BACKUP) {
        say("Save file was missing or corrupt; loaded from backup.\n");
    }// if 

    mainloop();

    return 0;
}/* main */


