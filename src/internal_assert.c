// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "ui.h"

#include "internal_assert.h"


void
internal_assert(int expr, const char *file, int line, const char *exprDesc,
                const char *msg, bool coredump) {
    if (expr) return;

    teardown_ui();
    
    notify("FATAL ERROR: %s:%d (%s) %s\n", file, line, exprDesc, msg);

    if (coredump) {
        abort();
    } else {
        exit(1);
    }// if .. else
}// internal_assert

