// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This module creates and sends the humourous(?) emails the user gets
// upon winning the game.  (These days, "sending" means "appending to
// a folder in ~/.relarnrc").

#ifndef HDR_GUARD_BILL_H
#define HDR_GUARD_BILL_H

#include <stdbool.h>

void load_email_templates(void);
bool write_emails(void);
void launch_client(void);

#endif
