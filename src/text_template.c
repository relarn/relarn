// This file is part of ReLarn; Copyright (C) 1986 - 2023; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "text_template.h"

#include "game.h"
#include "constants.h"
#include "stringbuilder.h"

#include <time.h>

static void append_expansion(struct StringBuilder *result,
                             const struct Player *pl,
                             const char *keyword);


char *
text_expand(char *template, const struct Player *pl) {
    struct StringBuilder *result = sb_alloc();
    struct StringBuilder *keyword = sb_alloc();

    for (size_t n = 0; template[n]; n++) {

        // $$ escapes to '$'
        if (template[n] != '$' ||
            (template[n] == '$' && template[n + 1] == '$'))
        {
            sb_append_char(result, template[n]);
            continue;
        }

        // If there's no brace, it's not a keyword and we advance.
        if (template[n+1] != '{') { continue; }
        n += 2;     // Skip past the opening brace

        sb_reset(keyword);
        while (true) {
            if (template[n] == '}') { break; }
            if (!template[n]) {
                sb_reset(keyword);
                break;
            }

            sb_append_char(keyword, template[n]);
            n++;
        }// for
        if (sb_len(keyword) == 0) { continue; }

        append_expansion(result, pl, sb_str(keyword));
    }// for

    sb_free(keyword);

    return sb_str_and_free(result);
}// text_expand


// Format a number as an amount of gold pieces
const char *
gold_gp(long amount) {
    static char text[40];
    snprintf(text, sizeof(text), "%ld gold piece%s", amount,
             amount == 1 ? "" : "s");
    return text;
}// gold_gp


static void
append_expansion(struct StringBuilder *result, const struct Player *pl,
                 const char *keyword) {
    if (streq("player", keyword)) {
        sb_append(result, pl->name);
    }
    else if (streq("player_email", keyword)) {
        char email[PLAYERNAME_MAX];
        zstrncpy(email, pl->name, sizeof(email));
        for (size_t n = 0; email[n]; n++) {
            email[n] = tolower(email[n]);
            if (!isalnum(email[n])) { email[n] = '.'; }
        }// for

        sb_append(result, email);
        sb_append(result, "@geemail.com.ln");
    }
    else if (streq("score", keyword)) {
        long score = compute_score(true);

        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%ld", score);

        sb_append(result, buffer);
    }
    else if (streq("taxes_owed_gp", keyword)) {
        sb_append(result, gold_gp(compute_taxes_owed(pl)));
    }
    else if (streq("wealth_gp", keyword)) {
        sb_append(result, gold_gp(pl->gold + pl->bankaccount));
    }
    else if (streq("woman", keyword)) {
        sb_append(result, woman(pl->gender));
    }
    else if (streq("date", keyword)) {
        time_t tm;
        time(&tm);

        char buffer[40];
        zstrncpy(buffer, ctime(&tm), sizeof(buffer));
        buffer[ strlen(buffer) - 1 ] = 0;   // Remove trailing newline
        
        sb_append(result, buffer);
    }
    else if (streq("slaying", keyword)) {
        sb_append(result, pl->killedBigBad ? "slaying" : "evading");
    }
    else {
        sb_append(result, "[REDACTED]");
    }
}// append_expansion
