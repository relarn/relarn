
#include "bill.h"

#include "internal_assert.h"
#include "stringbuilder.h"
#include "text_template.h"
#include "game.h"
#include "settings.h"
#include "os.h"
#include "ui.h"

#include "time.h"


static char **templates = NULL;
static int num_templates = 0;


// Load the mail templates, exiting the program on error.
//
// Note: also not very tolerant of formatting errors in the input file.
void
load_email_templates() {
    FILE *fh = fopen(junkmail_path(), "r");
    ENSURE_MSG(fh, "Unable to load email template file.");

    struct StringBuilder *sb = sb_alloc();
    while(true) {
        char line[200];
        char *stat = fgets(line, sizeof(line), fh);
        line[sizeof(line) - 1] = 0; // ensure null-termination

        if (sb_len(sb) > 0 && (line[0] == '\f' || !stat)) {
            ++num_templates;
            templates = xrealloc(templates, num_templates*sizeof(char *));
            templates[num_templates - 1] = sb_str_and_free(sb);

            if (!stat) {
                ENSURE_MSG(feof(fh),
                           "Error occurred while reading email templates.");
                break;
            }

            sb = sb_alloc();
            continue;
        }// if

        sb_append(sb, line);
    }// while

    fclose(fh);
}// load_emails



bool
write_emails() {
    bool status = true;

    FILE *fh = fopen(mailfile_path(), "a");
    if (!fh) { return false; }

    lock_file(fh);  // Just assume it worked 'cuz what else can we do?

    // It turns out that neomutt (and other clients?) use the date
    // in the separator line as the message date, so we put the
    // date here as well.
    char *sep = text_expand("From RELARN ${date}", &UU);

    for (int n = 0; n < num_templates; n++) {
        // Player always gets the first two in normal play; the rest
        // are up to chance.
        if (n > 1 && !UU.wizardMode && rnd(10) > 6) { continue; }

        // Print mbox separator
        int count1 = fprintf(fh, "%s\n", sep);

        // And print the message
        char *msg = text_expand(templates[n], &UU);
        int count2 = fprintf(fh, "%s\n", msg);
        free(msg);

        // Track if anything fails
        status = status && count1 >= 0 && count2 >= 0;
    } // for

    unlock_file(fh);
    fclose(fh);

    free(sep);

    return status;
}// write_emails



static void
make_mail_command(char dest[], size_t dest_len) {

    // Signal an unset mail client my making 'dest' empty as well.
    if (!GameSettings.emailClient[0]) {
        ASSERT(dest_len > 0);
        dest[0] = 0;
    }// if

    // First, make a scratch copy of option.
    char buffer[MAX_CMDLINE_LENGTH];
    strncpy(buffer, GameSettings.emailClient, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = 0;

    // Find the %s.  If there isn't one, add it (quoted).
    char *here = strstr(buffer, "%s");
    if (!here) {
        strncat(buffer, " \"%s\"", sizeof(buffer) - strlen(buffer) - 1);
        buffer[sizeof(buffer) - 1] = 0;

        // Advance 'here' to the '%', now 3 characters from the end of
        // the string.  (This will be wrong--but fail below--if we ran
        // out of buffer above.)
        here = buffer + strlen(buffer) - 3;
    }

    // Now, we split buffer into two strings; the part before the %s
    // and the part after.
    *here = 0;
    char *front = buffer;
    char *back = here + 2;

    snprintf(dest, dest_len, "%s%s%s", front, mailfile_path(), back);

    // If we've run out of command line, (i.e. we've filled the
    // buffer), complain to the user.
    if (strlen(dest) == dest_len) {
        dest[0] = 0;
        notify("Can't run email client; command is too long.");
    }
}// make_mail_command


// Launch the email client on exit if one is configured; otherwise
// prints a biff-style "you have mail" message.  Assumes that UI has
// shut down.
void
launch_client() {
    bool runClient = true;

    if (!GameSettings.emailClient[0]) {
        runClient = false;
    }// if

    // Create the complete command line
    char cmd[MAXPATHLEN + 2 + MAX_CMDLINE_LENGTH];
    make_mail_command(cmd, sizeof(cmd));
    if (!cmd[0]) {
        runClient = false;
    }

    // Run the client if everything is okay.
    if (runClient) {
        int status = system(cmd);
        runClient = (status == 0);     // Failed if exit status is not 0
    }// if

    // On failure or no client, report new mail.
    if (!runClient) {
        notify("You have new mail in %s", mailfile_path());
    }// if
}// launch_client
