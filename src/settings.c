// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "settings.h"


#include "os.h"


struct Options GameSettings;

void
initopts() {
    memset(&GameSettings, 0, sizeof(GameSettings));

    strncpy(GameSettings.name, get_username(),
            sizeof(GameSettings.name) - 1);
    GameSettings.name[0] = toupper(GameSettings.name[0]);   
    GameSettings.name[sizeof(GameSettings.name) - 1] = 0;
}/* initopts*/


// Strip leading and trailing whitespace.  Return NULL if the line is
// blank or a comment.
static char *
tidy_line(char *line) {
    /* Skip any leading whitespace. */
    while (*line && isspace(*line)) {
        ++line;
    }/* while */

    /* Skip any blank lines or comments. */
    if (!*line || *line == '#') { return NULL; }

    /* Lop off any trailing whitespace. */
    char *end = &line[strlen(line) - 1];
    while (end > line && isspace(*end)) {
        *end = '\0';
        --end;
    }/* while */

    return line;
}// tidy_line


static bool
opt(const char *line, const char *opt, char **arg) {
    size_t olen = strlen(opt);

    // Return false if this option line does not contain 'opt'
    if (strncmp(line, opt, olen) != 0) { return false; }

    // Return true if this option doesn't require an argument.
    if(opt[olen - 1] != ':' || !arg) { return true; }

    // Skip the leading spaces from the argument
    const char *argptr = line + olen;
    while(*argptr && isspace(*argptr)) {
        ++argptr;
    }// while

    // Set the argument and return.
    if (arg) { *arg = (char *)argptr; }
    return true;
}// opt


/*
 *  function to read and process the larn options file
 */
void
readopts (const char *opts_file) {
    char buffer[255];
    FILE *fp;

    fp = fopen(opts_file, "r");
    if (!fp) return;

    while (1) {
        char *line, *arg = NULL;

        line = fgets(buffer, sizeof(buffer), fp);
        if (!line) break;

        line = tidy_line(line);
        if (!line) { continue; }

        if (opt(line, "bold-off", NULL) || opt(line, "no-bold", NULL)) {
            printf("Option '%s' is obsolete; ignored.\n", line);
            continue;
        }/* if */
        
        if (opt(line, "female", NULL)) {
            GameSettings.sex = FEMALE;
            GameSettings.sexSet = true;
            continue;
        }/* if */

        if (opt(line, "male", NULL)) {
            GameSettings.sex = MALE;
            GameSettings.sexSet = true;
            continue;
        }

        if (opt(line, "name:", &arg)) {
            strncpy(GameSettings.name, arg, sizeof(GameSettings.name));
            continue;
        }

        if (opt(line, "no-introduction", NULL)) {
            GameSettings.nointro = true;
            continue;
        }/* if */

        if (opt(line, "no-beep", NULL)) {
            GameSettings.nobeep = true;
            continue;
        }/* if */

        if (opt(line, "email-client:", &arg)) {
            strncpy(GameSettings.emailClient, arg,
                    sizeof(GameSettings.emailClient) - 1);
            continue;
        }/* if */

        if (opt(line, "no-nap", NULL)) {
            GameSettings.nonap = true;
            continue;
        }/* if */

        if (opt (line, "character:", &arg)) {
            GameSettings.cclass = ccvalue(arg);
            if (GameSettings.cclass == CCNONE) {
                printf("Invalid character class name '%s'.\n", arg);
            }// if 
            continue;
        }/* if */
        
        // Unknown token:
        printf("Unknown config option: '%s'\n", line);
    }/* while */
}/* readopts */
