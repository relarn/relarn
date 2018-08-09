// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "score_file.h"

#include "internal_assert.h"

#include "game.h"

#include "settings.h"
#include "os.h"


//#define SCOREFILE SCOREBOARDDIR "/" SCORENAME
#define SCORE_LINE_MAX 300

struct ScoreBoardEntry {
    long        uid;                    // the user id number of the player
    long        won;                    // did the player win?  1 = yes, 0 = no
    long        score;                  // the score of the player
    long        taxes;                  // taxes owed for this game
    long        challenge;              // the level of difficulty
    long        level;                  // final cave level
    long        exp_level;              // final experience level
    long        sex;                    // Character gender
    char        who[PLAYERNAME_MAX];    // the name of the character        
    char        cclass[20];             // the character class
    char        ending[80];             // how the player met their end
};


/* Create the scoreboard if it's not there.*/
void
ensureboard() {
    if (freadable(scoreboard_path())) {
        return;
    }/* if */

    FILE *fh = fopen(scoreboard_path(), "w");
    ENSURE_MSG(fh, "Unable to write to scorefile.");
    fclose(fh);

    // We're going to test locking here and then just sort of assume
    // it'll work from now on.  (I wouldn't do this for important
    // software, but this is a game's scorefile so it's good enough.)
    LOCK_HANDLE lh = lock_file(scoreboard_path());
    ENSURE_MSG(lock_success(lh), "Unable to lock scorefile.");
    unlock_file(lh);
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
             "%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t%s\t%s\t%s\n",
             dest->uid,
             dest->won,
             dest->score,
             dest->taxes,
             dest->challenge,
             dest->level,
             dest->exp_level,
             dest->sex,
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
    } outputs[] = { {true,   &dest->uid,        sizeof(long)},
                    {true,   &dest->won,        sizeof(long)},
                    {true,   &dest->score,      sizeof(long)},
                    {true,   &dest->taxes,      sizeof(long)},
                    {true,   &dest->challenge,  sizeof(long)},
                    {true,   &dest->level,      sizeof(long)},
                    {true,   &dest->exp_level,  sizeof(long)},
                    {true,   &dest->sex,        sizeof(long)},
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

    FILE *fh = fopen(scoreboard_path(), "rb");
    if (!fh) { return NULL; }

    LOCK_HANDLE lh = lock_file(scoreboard_path());
    
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

    fclose(fh);
    unlock_file(lh);

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

    sb.uid = get_user_id();
    strncpy(sb.who, uu->name, sizeof(sb.who));
    strncpy(sb.cclass, ccname(uu->cclass), sizeof(sb.cclass));
    sb.sex = uu->sex;
    sb.won = won;
    sb.score = score;
    sb.taxes = compute_taxes_owed(uu);
    sb.challenge = uu->challenge;
    sb.level = level;
    sb.exp_level = uu->level;
    strncpy(sb.ending, ending, sizeof(sb.ending));

    // strncpy doesn't null-terminate if the source is longer than the
    // maximum.
    sb.who[sizeof(sb.who) - 1] = 0;
    sb.cclass[sizeof(sb.cclass) - 1] = 0;
    sb.ending[sizeof(sb.ending) - 1] = 0;

    // Sanitize the strings
    sanitize(&sb);

    LOCK_HANDLE lh = lock_file(scoreboard_path());
    
    FILE *fh = fopen(scoreboard_path(), "a");
    if (!fh) { return false; }

    int status = fputs(encode(&sb), fh);
    fclose(fh);

    unlock_file(lh);
    
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

    const long userid = get_user_id();
    
    for (size_t n = 0; n < len; n++) {
        if (items[n].uid == userid && items[n].won) {
            taxes = items[n].taxes;
            break;
        }// if 
    }// for 

    free(items);
    return taxes;
}// get_taxes_owed


void
showscores(bool all) {
    size_t len = 0;
    struct ScoreBoardEntry *items = load_scorefile(&len);

    if (!items) {
        printf(len == 0 ? "No scores yet.\n" : "Score file is corrupt!\n");
        return;
    }// if 


    printf("%-9s %-5s %-9s %-3s %-5s\n", "Score", "", "Challenge", "Floor", "Level");
    
    size_t n;
    for (n = 0; n < len; n++) {
        struct ScoreBoardEntry item = items[n];

        if (!all && !item.won) { continue; }
        
        char nbuf[sizeof(item.who) + sizeof(item.cclass) + 60];
        snprintf(nbuf, sizeof(nbuf), "%s the %s %s", item.who,
                 item.sex == 0 ? "female" : "male", item.cclass);

        printf("%9ld %-5s %-9d %-5d %-5d\n%s, you %s\n",
               item.score,
               (all && item.won) ? "(won)" : "",
               (int)item.challenge + 1,
               (int)item.level,
               (int)item.exp_level,
               nbuf,
               item.ending);
    }// for 
}/* showscores*/




