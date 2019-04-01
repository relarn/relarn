// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "settings.h"


#include "os.h"


struct Options GameSettings;

void
initopts() {
    memset(&GameSettings, 0, sizeof(GameSettings));

    // Default name:
    strncpy(GameSettings.name, get_username(),
            sizeof(GameSettings.name) - 1);
    GameSettings.nameSet = false;
    
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



static bool
gender(const char *line, const char *prefix, enum GENDER *gender,
       bool *genderSet)
{
    static const char *strings[] = GENDER_STRINGS;
    for(int n = 0; n < 3; n++) {
        char desc[20];
        snprintf(desc, sizeof(desc), "%s%s", prefix, strings[n]);

        if (opt(line, desc, NULL)) {
            *gender = (enum GENDER)n;
            *genderSet = true;
            return true;
        }// if
    }// for

    return false;
}// gender



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
        
        if (gender(line, "", &GameSettings.gender, &GameSettings.genderSet)) {
            continue;
        }/* if */

        if (gender(line, "spouse_", &GameSettings.spouseGender,
                   &GameSettings.spouseGenderSet)) {
            continue;
        }/* if */

        if (opt(line, "name:", &arg)) {
            strncpy(GameSettings.name, arg, sizeof(GameSettings.name));
            GameSettings.nameSet = true;
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

        if (opt(line, "show-fov", NULL)) {
            GameSettings.showFoV = true;
            continue;
        }/* if */

        if (opt(line, "show-unrevealed", NULL)) {
            GameSettings.showUnrevealed = true;
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
