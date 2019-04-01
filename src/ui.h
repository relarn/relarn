// This file is part of ReLarn; Copyright (C) 1986 - 2019; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// This is the abstraction layer around the user interface.  The
// current implementation is curses-based but it's easy enough (I
// hope) to swap in some other UI system.


#ifndef HDR_GUARD_UI_H
#define HDR_GUARD_UI_H

#include <stdlib.h>

#include "textbuffer.h"
#include "picklist.h"
#include "player.h"


#define SCREEN_W 80
#define SCREEN_H 25


enum INDICATOR {
    IND_STEALTH     = 0,
    IND_UNDEAD_PRO  = 1,
    IND_SPIRIT_PRO  = 2,
    IND_CHARM       = 3,
    IND_TIME_STOP   = 4,
    IND_HOLD_MONST  = 5,
    IND_GIANT_STR   = 6,
    IND_FIRE_RESIST = 7,
    IND_DEXTERITY   = 8,
    IND_STRENGTH    = 9,
    IND_SCARE       = 10,
    IND_HASTE_SELF  = 11,
    IND_CANCEL      = 12,
    IND_INVISIBLE   = 13,
    IND_PROTECT_3   = 14,
    IND_PROTECT_2   = 15,
    IND_WALLWALK    = 16,

    IND_MAX,
};

enum MAPFLAGS {
    MF_DEFAULT,     /* Standard map square. */
    MF_OBJ,         /* Most objects.  Typically reverse video. */
    MF_EFFECT,      /* Special effects (e.g. magical bolts). */
    MF_PLAYER,      /* The player.  Highlighted somehow. */
    MF_PLAYER_INV,  /* The player, but invisible.  Different color. */
    MF_NOTSEEN,     /* This cell has not been explored yet. */
    MF_FOV,         /* This cell is within the player's field of view. */
};



void init_ui(void);
void teardown_ui(void);
void sync_ui(bool force);

char map_getch(void);


void showstats(const struct Player *p, bool iswiz, bool force);
void mapdraw(int x, int y, char symbol, enum MAPFLAGS flags);
void say(const char *fmt, ...);
void show_indicators(const struct Player *uu, bool force);

char menu(const char *heading, const char* items);

void showpages(struct TextBuffer *tb);
bool showpages_prompt(struct TextBuffer *tb, bool prompt);
bool pick_item (struct PickList *tb, const char *heading, int *id);
int pick_multi (struct PickList *pl, const char *heading,int **ids,bool multi);
bool get_player_type(char *, size_t, enum CHAR_CLASS *, enum GENDER *,
                     enum GENDER *);
char prompt(const char *question);
bool confirm(const char *question);
bool stringPrompt(const char *question, char *result, size_t maxSize);
long numPrompt(const char *question, long defaultValue, long max);
long numPromptAll(const char *question, long defaultValue, long min, long max,
                  bool *success);
void promptToContinue(void);

char quickinv(const char *action, const char *candidates, bool dashForNone,
              bool allowGold);
DIRECTION promptdir(bool allowCancel);
void billboard(bool center, const char *heading, ...);
void say(const char *fmt, ...);

void nap(int x);

void headsup(void);

#endif

