// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


#include "internal_assert.h"

#include "picklist.h"


/* Create and initialize a PickList struct. */
struct PickList *
pl_malloc() {
    struct PickList *pl;

    pl = xmalloc (sizeof(struct PickList));

    memset (pl, 0, sizeof (struct PickList));
    return pl;
}/* pl_malloc*/


void
pl_free(struct PickList *pl) {
    free (pl);
}/* pl_free*/

/* Add an entry to 'pl' with id 'id' and description 'desc'.  'letter' may
 * be 0 if no character is to be displayed.  Otherwise, it must be a
 * letter. */
void
pl_add(struct PickList *pl, int id, char letter, const char *desc) {
    int ndx;

    ASSERT (letter == 0 || isalpha(letter));
    ASSERT (strlen(desc) < DESC_MAX);

    if (pl->num_elems + 1 >= MAX_ELEM) {
        /* MAX_ELEM is too small.  This is an internal error.*/
        ASSERT (pl->num_elems + 1 < MAX_ELEM);
        return;
    }/* if */

    ndx = pl->num_elems;
    pl->items[ndx].id = id;
    pl->items[ndx].letter = letter;
    strncpy (pl->items[ndx].description, desc, DESC_MAX);

    ++pl->num_elems;
}/* pl_add*/


/* Look up the entry associated with 'letterSought' and store its id
 * value in the int pointed at by 'id'.  If absent, return false;
 * otherwise return true. */
bool
pl_lookup (struct PickList *pl, int letterSought, int *id) {
    int n;

    for (n = 0; n < pl->num_elems; n++) {
        if (pl->items[n].letter == letterSought) {
            *id = pl->items[n].id;
            return true;
        }/* if */
    }/* for */

    return false;
}/* pl_lookup */
