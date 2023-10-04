// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "score_file.h"

#include "internal_assert.h"

#include "game.h"
#include "ui.h"

#include "settings.h"
#include "os.h"


//#define SCOREFILE SCOREBOARDDIR "/" SCORENAME
#define SCORE_LINE_MAX 300

struct ScoreBoardEntry {
    char        uid[OS_UID_STR_MAX+1];  // OS-specific user id rendered as a string
    long        won;                    // did the player win?  1 = yes, 0 = no
    long        score;                  // the score of the player
    long        taxes;                  // taxes owed for this game
    long        challenge;              // the level of difficulty
    long        level;                  // final cave level
    long        exp_level;              // final experience level
    long        gender;                 // Character gender
    char        who[PLAYERNAME_MAX];    // the name of the character
    char        cclass[20];             // the character class
    char        ending[80];             // how the player met their end
};


// Ensure we can write to the scoreboard file.  Also creates it if it
// doesn't exist.
void
ensureboard() {
    const char *sbpath = scoreboard_path();

    FILE *fh = fopen(sbpath, "ab");
    ENSURE_MSG(fh, "Unable to write to scorefile.");

    // We're going to test locking here and then just sort of assume
    // it'll work from now on.  (I wouldn't do this for important
    // software, but this is a game's scorefile so it's good enough.)
    ENSURE_MSG(lock_file(fh), "Unable to lock scorefile.");
    unlock_file(fh);

    fclose(fh);
}/* ensureboard*/


// Replace all tabs or newlines in dest with spaces so that they don't
// mess up the parsing.
static void
sanitize(struct ScoreBoardEntry* dest) {
    ws_to_space(dest->who);
    ws_to_space(dest->cclass);
    ws_to_space(dest->ending);
}// sanitize


static char *
encode(const struct ScoreBoardEntry* dest) {
    static char buffer[SCORE_LINE_MAX];

    snprintf(buffer, sizeof(buffer),
             "%s\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%s\t%s\t%s\n",
             dest->uid,
             dest->won,
             dest->score,
             dest->taxes,
             dest->challenge,
             dest->level,
             dest->exp_level,
             dest->gender,
             dest->who,
             dest->cclass,
             dest->ending);

    ASSERT(strlen(buffer) < sizeof(buffer));

    return buffer;
}// encode



// Decode 'line' into 'dest'.  Returns true on success, false if an
// error occurred.
//
// WARNING: modifies argument 'line'.
static bool
decode(char *line, struct ScoreBoardEntry* dest) {

    char *pos;
    char *lp = line;

    struct {
        bool is_num;
        void *outptr;
        size_t max;
    } outputs[] = { {false,  &dest->uid,        sizeof(dest->uid)},
                    {true,   &dest->won,        sizeof(long)},
                    {true,   &dest->score,      sizeof(long)},
                    {true,   &dest->taxes,      sizeof(long)},
                    {true,   &dest->challenge,  sizeof(long)},
                    {true,   &dest->level,      sizeof(long)},
                    {true,   &dest->exp_level,  sizeof(long)},
                    {true,   &dest->gender,     sizeof(long)},
                    {false,  dest->who,         sizeof(dest->who)},
                    {false,  dest->cclass,      sizeof(dest->cclass)},
                    {false,  dest->ending,      sizeof(dest->ending)},
                    {false,  NULL,              0} };
    for (int n = 0; outputs[n].outptr; n++) {
        char *val = strtok_r(lp, "\t", &pos);
        if (!val) { return false; }
        lp = NULL;

        if (outputs[n].is_num) {
            char *end = "x";
            long *lop = (long *)outputs[n].outptr;
            *lop = strtol(val, &end, 10);
            if (*end != 0) { return false; }
        } else {
            char *op = (char *)outputs[n].outptr;
            strncpy(op, val, outputs[n].max);
            op[outputs[n].max - 1] = 0;  // Ensure null-term.
        }// if .. else
    }// for

    return true;
}// decode

// Comparision function for sorting the list of entries into
// descending order
static int
cmp_sbe(const void *va, const void *vb) {
    const struct ScoreBoardEntry *a = va, *b = vb;

    if (a->score < b->score) { return 1; }
    if (a->score > b->score) { return -1; }
    return 0;
}// cmp_sbe


// Return the scorefile as an array of structs.  *count is set to the
// number of items.
//
// On error, *count is set to a non-zero value to distinguish between
// an empty scorefile.
static struct ScoreBoardEntry *
load_scorefile(size_t *count) {
    *count = 0;

    FILE *fh = fopen(scoreboard_path(), "r+b");
    if (!fh) { return NULL; }

    lock_file(fh);

    struct ScoreBoardEntry *result = NULL;
    size_t num_entries = 0;
    bool error = false;

    for (int n = 0; ; n++) {
        struct ScoreBoardEntry item;
        char line[SCORE_LINE_MAX];

        char *ok = fgets(line, sizeof(line), fh);
        if (!ok) {
            if (feof(fh)) { break; }
            error = true;
            break;
        }

        if (!decode(line, &item)) {
            error = true;
            break;
        }

        ++num_entries;
        result = xrealloc(result, num_entries * sizeof(struct ScoreBoardEntry));
        memcpy(&result[num_entries - 1], &item, sizeof(struct ScoreBoardEntry));
    }// for

    unlock_file(fh);
    fclose(fh);

    if (error) {
        free(result);
        *count = 42;
        return NULL;
    }

    qsort(result, num_entries, sizeof(struct ScoreBoardEntry), cmp_sbe);

    *count = num_entries;
    return result;
}// load_scorefile



// Append a new score to the scorefile.
bool
newscore(long score, bool won, int level, const char *ending,
         const struct Player *uu) {
    struct ScoreBoardEntry sb;

    zstrncpy(sb.uid, get_user_id(), OS_UID_STR_MAX);
    zstrncpy(sb.who, uu->name, sizeof(sb.who));
    zstrncpy(sb.cclass, ccname(uu->cclass), sizeof(sb.cclass));
    sb.gender = uu->gender;
    sb.won = won;
    sb.score = score;
    sb.taxes = compute_taxes_owed(uu);
    sb.challenge = uu->challenge;
    sb.level = level;
    sb.exp_level = uu->level;
    zstrncpy(sb.ending, ending, sizeof(sb.ending));

    // Sanitize the strings
    sanitize(&sb);

    FILE *fh = fopen(scoreboard_path(), "ab");
    if (!fh) { return false; }
    lock_file(fh);

    int status = fputs(encode(&sb), fh);

    unlock_file(fh);
    fclose(fh);

    return status >= 0;
}/* newscore*/


// Retrieve taxes owed from the scoreboard.
long
get_taxes_owed() {
    long taxes = -1;
    if (taxes >= 0) { return taxes; }

    size_t len = 0;
    struct ScoreBoardEntry *items = load_scorefile(&len);
    if (!items) {
        return 0;
    }// if

    const char * userid = get_user_id();

    for (size_t n = 0; n < len; n++) {
        if (streq(items[n].uid, userid) && items[n].won) {
            taxes = items[n].taxes;
            break;
        }// if
    }// for

    free(items);
    return taxes;
}// get_taxes_owed


void
showscores(bool all) {
    char buffer[200];


    struct TextBuffer *dest = tb_malloc(INF_BUFFER, SCREEN_W);
    
    size_t len = 0;
    struct ScoreBoardEntry *items = load_scorefile(&len);

    // Heading
    snprintf(buffer, sizeof(buffer),
             "%-9s %-5s %-9s %-3s %-5s\n",
             "Score", "", "Challenge", "Floor", "Level");
    tb_append(dest, buffer);

    if (!items && len > 0) {
        tb_append(dest, "\n");
        tb_append(dest, "    *** Error reading score file ***");
        len = 0;
    }

    size_t n;
    for (n = 0; n < len; n++) {
        struct ScoreBoardEntry item = items[n];

        if (!all && !item.won) { continue; }

        char nbuf[sizeof(item.who) + sizeof(item.cclass) + 60];
        snprintf(nbuf, sizeof(nbuf), "%s the %s %s", item.who,
                 female((enum GENDER)item.gender), item.cclass);

        snprintf(buffer, sizeof(buffer),
                 "%9ld %-5s %-9d %-5d %-5d\n%s, you %s\n",
                 item.score,
                 (all && item.won) ? "(won)" : "",
                 (int)item.challenge + 1,
                 (int)item.level,
                 (int)item.exp_level,
                 nbuf,
                 item.ending);
        tb_append(dest, buffer);
    }// for

    showpages(dest);

    tb_free(dest);
}/* showscores*/



// Write the scores to a file (or stdout where supported).
bool
write_scores(const char *filename) {
    FILE *fh = NULL;

    if (streq(filename, "-")) {
        fh = stdout;
    } else {
        fh = fopen(filename, "w");
        if (!fh) {
            return false;
        }// if
    }// if .. else

    fprintf(fh,
            "# name\tgender\tclass\tscore\twon?\tskill\tfloor\t"
            "exp\tending description\n");

    size_t len = 0;
    struct ScoreBoardEntry *items = load_scorefile(&len);

    size_t n;
    for (n = 0; n < len; n++) {
        struct ScoreBoardEntry item = items[n];

        fprintf(fh,

                "%s\t%s\t%s\t" "%ld\t%s\t" "%d\t%d\t%d\t%s",

                item.who,
                female((enum GENDER)item.gender),
                item.cclass,

                item.score,
                item.won ? "won" : "failed",

                (int)item.challenge + 1,
                (int)item.level,
                (int)item.exp_level,
                item.ending);
    }// for

    free(items);
    
    if (fh != stdout) {
        fclose(fh);
    }// if

    return true;
}// write_scores
