
#include "bill.h"

#include "internal_assert.h"
#include "stringbuilder.h"
#include "text_template.h"
#include "game.h"
#include "settings.h"
#include "os.h"




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
    LOCK_HANDLE lh = lock_file(mailfile_path());
   
    FILE *fh = fopen(mailfile_path(), "a");
    if (!fh) {
        printf("Error delivering your email: mailer daemon got exorcise.\n");
        status = false;
        goto done;
    }

    for (int n = 0; n < num_templates; n++) {
        // Player always gets the first two in normal play; the rest
        // are up to chance.
        if (n > 1 && !GS.wizardMode && rnd(10) > 6) { continue; }

        // Don't bother with a meaningful address or date in the
        // separator; just somethig that works as a divider.  (Well,
        // the date *is* meaningful; just not wrt. the current game.)
        fprintf(fh, "From relarn@not.an.address Sun Aug 26 20:57:08 1991\n");

        // And print the message
        fprintf(fh, "%s\n", text_expand(templates[n], &UU));
    } // for 
    
    fclose(fh);

done:
    unlock_file(lh);
    return status;
}// write_emails


// Launch the email client on exit if one is configured; otherwise
// prints a biff-style "you have mail" message.  Assumes that UI has
// shut down.
void
launch_client() {
    bool runClient = true;
    
    if (!GameSettings.emailClient[0]) {
        runClient = false;
    }// if

    char cmd[MAXPATHLEN + 2 + MAX_CMDLINE_LENGTH];
    snprintf(cmd, sizeof(cmd), "%s \"%s\"", GameSettings.emailClient,
             mailfile_path());
    if (strlen(cmd) + 1 >= sizeof(cmd)) {   // Pretty unlikely
        runClient = false;
    }// if

    if (runClient) {
        int status = system(cmd);
        runClient = (status == 0);     // Failed if exit status is not 0
    }// if

    if (!runClient) {
        printf("You have new mail in %s\n", mailfile_path());
    }// if 
}// launch_client

