// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Code for showing and selecting items in inventory.

#ifndef HDR_GUARD_SHOW_H
#define HDR_GUARD_SHOW_H

#include <stdbool.h>

/* show.c */
enum PRICEMODE {
    PRM_NONE        = 0,
    PRM_STORE,
    PRM_BANK,
};
   

int inv_pick(const char *desc, unsigned filter, enum PRICEMODE pricemode);
int inv_pick_multi(const char *desc, unsigned filter, enum PRICEMODE pricemode,
                   int **ids, bool multi);
void show_inv_item(int index);
void list_known(void);
const char *inv_line(int index, enum PRICEMODE pricemode);

#endif
