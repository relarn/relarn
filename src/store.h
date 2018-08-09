// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Holds the state and support code for common shop-like behaviour as
// well as the simpler shops.

#ifndef HDR_GUARD_STORE_H
#define HDR_GUARD_STORE_H

#include "object.h"
#include "show.h"

struct StoreItem {
    int    price;
    struct Object item;
    uint8_t    qty;
};

extern struct StoreItem ShopInvent[];
extern unsigned ShopInventSz;

void sell_multi(int filter, enum PRICEMODE pricemode, const char *intro,
                const char *nothing_to_sell, const char *nosale,
                const char *fmt_sold_for_price);
void dndstore(void);
void otradepost(void);
void olrs(void);
void opad(void);
void initstore(void);

#endif
