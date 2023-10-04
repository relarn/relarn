// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "settings.h"


#include "os.h"
#include "ui.h"

struct Options GameSettings;

#define CFGERR "Config error: "
#define LINE_MAX 1024

void
initopts() {
    memset(&GameSettings, 0, sizeof(GameSettings));

    // Default name:
    zstrncpy(GameSettings.name, get_username(),
             sizeof(GameSettings.name));
    GameSettings.nameSet = false;

    GameSettings.name[0] = toupper(GameSettings.name[0]);
    GameSettings.name[sizeof(GameSettings.name) - 1] = 0;

    // Default font size (where supported).
    GameSettings.fontSize = 18;

    // Cool "new" features are now on by default.
    GameSettings.showFoV = true;
    GameSettings.showUnrevealed = true;

    // Changed stats are bolded for 4 turns
    GameSettings.hilightTime = 4;
    GameSettings.hilightReverse = false;
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


// Parse 'font_desc' (form: "<size>,<path>") and store the values in
// GameSettings.  If 'size' is missing, keep the existing value.
static bool
set_font_settings(const char *font_desc_orig) {
    char font_desc[LINE_MAX];
    zstrncpy(font_desc, font_desc_orig, sizeof(font_desc));

    // Find the separating comma if present
    int cma = 0;
    for (; font_desc[cma] && font_desc[cma] != ','; ++cma) { }

    bool no_comma = false;
    char *path = &font_desc[cma+1];

    if (!font_desc[cma]) {
        no_comma = true;
        path = font_desc;
    }// if

    // Set default size if requested
    int sz = atoi(font_desc);
    if (!no_comma && sz >= 0) {
        GameSettings.fontSize = sz;
    }// if

    // Copy over the path to the font file
    zstrncpy(GameSettings.fontPath, path, sizeof(GameSettings.fontPath));
    GameSettings.fontPath[sizeof(GameSettings.fontPath) - 1] = 0;

    return true;
}// set_font_settings


static bool
getclr(char first, char second, uint8_t *result) {

    if (!isxdigit(first) || !isxdigit(second)) { return false; }

    char clr[] = {first, second, 0};

    *result = (uint8_t) strtol(clr, NULL, 16);

    return true;
}// getclr


static bool
set_one_color(const char *desc, struct Color *dest) {
    if (strlen(desc) != 6) { return false; }

    bool success =
        getclr(desc[0], desc[1], &dest->r)      &&
        getclr(desc[2], desc[3], &dest->g)      &&
        getclr(desc[4], desc[5], &dest->b);

    if (success) {
        dest->isSet = true;
    }

    return success;
}// set_one_color


static bool
set_colors(const char *line) {
    char colors[LINE_MAX];
    zstrncpy(colors, line, sizeof(colors));

    struct Color *clrs[] = {
        &GameSettings.black,
        &GameSettings.red,
        &GameSettings.green,
        &GameSettings.yellow,
        &GameSettings.blue,
        &GameSettings.magenta,
        &GameSettings.cyan,
        &GameSettings.white,
        NULL
    };

    const char *sep = " \t";
    int index = 0;
    char *context = NULL;
    for (char *item = strtok_r(colors, sep, &context);
         item;
         item = strtok_r(NULL, sep, &context) )
    {
        if (streq(item, "-")) { continue; }

        if (!set_one_color(item, clrs[index])) {
            say(CFGERR "Invalid color value '%s'\n", item);
            return false;
        }// if

        ++index;
    }// for

    if (clrs[index]) {
        say(CFGERR "Too few colors in config line:\n'%s'\n", line);
        return false;
    }// if

    return true;
}// set_colours




/*
 *  function to read and process the larn options file
 */
void
readopts (const char *opts_file) {
    char buffer[LINE_MAX];
    FILE *fp;

    fp = fopen(opts_file, "r");
    if (!fp) return;

    bool gui_client_set = false;
    while (1) {
        char *line, *arg = NULL;

        line = fgets(buffer, sizeof(buffer), fp);
        if (!line) break;

        line = tidy_line(line);
        if (!line) { continue; }

        if (opt(line, "bold-off", NULL) || opt(line, "no-bold", NULL)) {
            say(CFGERR "Option '%s' is obsolete; ignored.\n", line);
            continue;
        }// if

        if (gender(line, "", &GameSettings.gender, &GameSettings.genderSet)) {
            continue;
        }// if

        if (gender(line, "spouse_", &GameSettings.spouseGender,
                   &GameSettings.spouseGenderSet)) {
            continue;
        }// if

        if (opt(line, "name:", &arg)) {
            zstrncpy(GameSettings.name, arg, sizeof(GameSettings.name));
            GameSettings.nameSet = true;
            continue;
        }

        if (opt(line, "no-introduction", NULL)) {
            GameSettings.nointro = true;
            continue;
        }// if

        if (opt(line, "no-beep", NULL)) {
            GameSettings.nobeep = true;
            continue;
        }// if

        if (opt(line, "email-client:", &arg)) {
            if(!gui_client_set) {
                zstrncpy(GameSettings.emailClient, arg,
                        sizeof(GameSettings.emailClient));
            }
            continue;
        }// if

        if (opt(line, "gui-email-client:", &arg)) {
            if(!is_tty()) {
                zstrncpy(GameSettings.emailClient, arg,
                        sizeof(GameSettings.emailClient));
                gui_client_set = true;
            }
            continue;
        }// if

        if (opt(line, "no-nap", NULL)) {
            GameSettings.nonap = true;
            continue;
        }// if

        if (opt(line, "show-fov", NULL)) {
            GameSettings.showFoV = true;
            continue;
        }// if

        if (opt(line, "no-show-fov", NULL)) {
            GameSettings.showFoV = false;
            continue;
        }// if

        if (opt(line, "show-unrevealed", NULL)) {
            GameSettings.showUnrevealed = true;
            continue;
        }// if

        if (opt(line, "no-show-unrevealed", NULL)) {
            GameSettings.showUnrevealed = false;
            continue;
        }// if

        if (opt (line, "character:", &arg)) {
            GameSettings.cclass = ccvalue(arg);
            if (GameSettings.cclass == CCNONE) {
                say(CFGERR "Invalid character class name '%s'.\n", arg);
            }// if
            continue;
        }// if

        if (opt (line, "difficulty:", &arg)) {
            int diff = atoi(arg);

            if (diff < 0 || diff > 100 || (diff == 0 && !streq("0", arg)) ) {
                say(CFGERR "Invalid difficulty value: '%s'\n", arg);
                diff = 0;
            }// if

            GameSettings.difficulty = diff;
            continue;
        }// if

        if (opt(line, "dark-screen", NULL)) {
            GameSettings.darkScreenSet = true;
            GameSettings.darkScreen = true;
            continue;
        }// if

        if (opt(line, "light-screen", NULL)) {
            GameSettings.darkScreenSet = true;
            GameSettings.darkScreen = false;
            continue;
        }// if

        if (opt(line, "font:", &arg)) {
            if (!set_font_settings(arg)) {
                say(CFGERR "Invalid font string in config file: '%s'\n", arg);
            }// if
            continue;
        }// if

        if (opt(line, "colors:", &arg)) {
            if (!set_colors(arg)) {
                say(CFGERR "Ignoring malformed 'colors:' line:\n\t%s\n", line);
            }// if
            continue;
        }// if

        if (opt (line, "hilight-persist:", &arg)) {
            int diff = max(0, atoi(arg));
            GameSettings.hilightTime = diff;
            continue;
        }

        if (opt(line, "hilight-reverse", NULL)) {
            GameSettings.hilightReverse = true;
            continue;
        }// if

        // Unknown token:
        say(CFGERR "Unknown config option: '%s'\n", line);
    }/* while */
}/* readopts */
