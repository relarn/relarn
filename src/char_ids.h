// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Header containing the definition of the enum type for character classes.

#ifndef HDR_CHAR_IDS_H
#define HDR_CHAR_IDS_H

enum CHAR_CLASS {
#   define CCLASS(id, desc) id,
#   include "char_classes.h"
#   undef CCLASS
    CC_COUNT,
    CC_LAST = CC_COUNT - 1,
};
//#define CC_LAST (CC_COUNT - 1);

#endif
