// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.


// Data structure representing a list of (possibly) selectable items
// intended for display and selection (e.g. inventory.)  Each item
// has a description, a letter (optional) and an identifier.  Entries
// can be looked up by searching for the associated letter if the
// letter is given.  Actual lookup and display is done by code
// in ui.h.
//
// We follow an OOP-ish pattern here, with the PickList struct as
// class and functions as methods.  Thus, the internals are considered
// private outside of this module and ui.c.


#ifndef HDR_GUARD_PICKLIST_H
#define HDR_GUARD_PICKLIST_H

#define MAX_ELEM 256
#define DESC_MAX 100        /* Max. length of a description. Should be
                             * longer than any object description. */

enum PICKLIST_FLAGS {
    PLF_DEFAULT     = 0x0,
    PLF_HIDELETTER  = 0x1,  /* Don't display item letter, even if given. */
};

struct PickList {
    struct {
        int id;
        char letter;
        char description[DESC_MAX];
    } items[MAX_ELEM];
    size_t num_elems;
    unsigned long flags;
};

struct PickList *pl_malloc(void);
void pl_free(struct PickList *);
void pl_add(struct PickList *, int id, char letter, const char *desc);

static inline void pl_setflags(struct PickList *pl, unsigned long flags) {
    pl->flags = flags;
}/* pl_setflags*/
static inline size_t pl_count(struct PickList *pl) {return pl->num_elems;}


#endif
