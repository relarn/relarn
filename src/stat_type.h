// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Stat type:
//  - split into base and modifier
//  - base is "permanent"; changed by "permanent" effects (e.g. fountains)
//  - modifier is ephemeral and recalculated each turn from inventory,
//    spells, statuses, etc.


#ifndef HDR_GUARD_STAT_TYPE_H
#define HDR_GUARD_STAT_TYPE_H

#include <stdint.h>

//#define STAT_MIN 3

struct Stat {
    // DO NOT modify these directly; instead, use the functions
    // defined below.
    int16_t base;       // The value of the stat
    int16_t mod;        // The modifier; this is changed by spells, items, etc.
    int16_t min_base;   // The minimum value the base can be set to
    int16_t min_sum;    // The minimum total value we can return
};

// Initialize 'stat'
inline static void stat_init(struct Stat *stat, int16_t base,
                             int16_t min_base, int16_t min_sum) {
    stat->base = base;
    stat->mod = 0;
    stat->min_base = min_base;
    stat->min_sum = min_sum;
}// init_stat

// Adjust the stat by 'val', then raise its value to minval if it is
// currently less.  This bypasses stat->min_base.
inline static void stat_adjust_min(struct Stat *stat, int16_t val,
                                   uint16_t minval) {
    stat->base += val;
    if (stat->base < minval) { stat->base = minval; }
}// stat_adjust

// Adjust the base of 'stat' by 'val', limited to a minimum of
// STAT_MIN.
inline static void stat_adjust(struct Stat *stat, int16_t val) {
    stat_adjust_min(stat, val, stat->min_base);
}// stat_adjust

// Retrieve the stat's value.  This is capped at stat->min_sum
inline static uint16_t stat_val(const struct Stat *stat) {
    return (uint16_t)max(stat->min_sum, stat->base + stat->mod);
}// stat_val

// Zero the modifier
inline static void stat_reset_mod(struct Stat *stat) {
    stat->mod = 0;
}// stat_reset_mod

// Update (i.e. add to) the modifier
inline static void stat_adjust_mod(struct Stat *stat, int16_t val) {
    stat->mod += val;
}// stat_set_offset


#undef STAT_MIN

#endif
