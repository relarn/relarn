// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "school.h"

#include "internal_assert.h"
#include "constants.h"


#define EDU_COST 250

struct Course {
    char *code;
    char *desc;
    char *message;
    int time;
    int prereq;
    char *effects;
};

static struct Course CourseList[EDU_LEVELS] = {
    {"PE 101", "Medieval Conflict Resolution",
     "You feel stronger!",
     10,     -1,     "s2o1"},
    
    {"PE 102", "Advanced Medieval Conflict Resolution",
     "You feel much stronger!",
     15,     0,      "s2o2"},
    
    {"CS 101", "Introduction to Wizardry",
     "The task before you now seems more attainable!",
     10,     -1,     "i2"},
    
    {"CS 102", "Applied Wizardry",
     "The task before you now seems very attainable!",
     20,     2,      "i2"},
    
    {"PY 101", "Behavioral Psychology",
     "You now feel like a born leader!",
     10,     -1,     "c3"},
    
    {"RS 101", "Faith for Today",
     "You feel much more confident!",
     10,     -1,     "w2"},
    
    {"PE 110", "Contemporary Dance",
     "You feel like dancing!",
     10,     -1,     "d3"},
    
    {"HS 101", "History of Larn",
     "Wow! e = mc^2!",
     5,      -1,     "i1"},
};


static int pick_course(void);
static void apply_effects(const char *effects);
static void elapse_time(int time);
static void take_course(int courseNum);

void
oschool() {
    int courseNum;
    struct Course *cp;

    if (UU.gold < EDU_COST) {
        say("You can't afford to take a course here.\n");
        return;
    }/* if */

    if (graduated(&UU)) {
        say("You've already graduated.\n");
        return;
    }/* if */
    
    courseNum = pick_course();
    if (courseNum < 0) return;
    ASSERT(courseNum < EDU_LEVELS);

    cp = &CourseList[courseNum];
    
    /* Ensure prerequisites have been met. */
    if (cp->prereq >= 0 && !UU.courses[cp->prereq]) {
        struct Course *pr = &CourseList[cp->prereq];
        say("Unfortunately, %s has a prerequisite, %s (%s).\n",
            cp->code, pr->code, pr->desc);
        return;
    }/* if */
    
    /* Actually take the course. */
    take_course(courseNum);

    /* Make the time pass */
    elapse_time(cp->time * MOBUL);

    /* And give the player a second to absorbe the knowledge. */
    nap(1000);
}/* oschool*/


/* Apply all the changes due to taking a course. */
static void
take_course(int courseNum) {
    struct Course *cp = &CourseList[courseNum];    

    /* Pay tuition. */
    UU.gold -= EDU_COST;

    /* Take the course */
    apply_effects(cp->effects);

    /*remember that the player has taken that course*/
    UU.courses[courseNum] = true;

    say("%s\n", cp->message);

    /* And notify the player if they are now a graduate. */
    if (graduated(&UU)) {
        say("Congratulations! You have graduated with a BH (Bachelor of "
            "Heroism).\n");
        struct Object diploma = obj(ODIPLOMA, 0);
        take(diploma,"Your diploma will be mailed to you in 600 to 800 mobuls");
    }/* if */
}/* take_course*/




/* Walk the 'effects' string in CourseList[] and apply its operations
 * to the player. */
static void
apply_effects(const char *effects) {
    const char *p;

    for (p = effects; *p; p += 2) {
        int offset = (int)*(p + 1) - '0';

        switch (*p) {
        case 's' : UU.strength += offset;       break;
        case 'o' : UU.constitution += offset;   break;
        case 'i' : UU.intelligence += offset;   break;
        case 'w' : UU.wisdom += offset;         break;
        case 'c' : UU.charisma += offset;       break;
        case 'd' : UU.dexterity += offset;      break;
        default: FAIL("Internal error: Invalid effect.");
        }/* switch */
    }/* for */
}/* apply_effects*/


/* Advance the clock by the given number of turns and update the
 * player's stats to reflect the passage of time.  This is not
 * entirely the same as waiting; some things don't elapse and bad
 * stuff is automatically timed out. */
static void 
elapse_time(int num_turns) {
    UU.gtime += num_turns;
        
    /* regenerate */
    UU.hp = UU.hpmax;
    UU.spells = UU.spellmax;
        
    /* cure blindness!  */
    if (UU.blindCount) {
        UU.blindCount = 1;
    }/* if */

    /*  end confusion   */
    if (UU.confuse) {
        UU.confuse = 1;
    }/* if */

    /* adjust parameters for time change */
    adjusttime(num_turns);
}/* elapse_time*/



/* Let the user select which course to take.  -1 == cancel.  Displays
 * a message if all courses have been taken and returns -1. */
static int
pick_course() {
    struct PickList *picker;
    int n, count, index;
    char *heading =
        "The University of Larn offers the exciting opportunity of higher "
        "education to\nall inhabitants of the caves.  Here is the class "
        "schedule:";

    picker = pl_malloc();
    
    count = 0;
    for (n = 0; n < EDU_LEVELS; n++) {
        char buffer[100];
        struct Course *cp = &CourseList[n];

        if (UU.courses[n]) continue;

        ++count;
        snprintf(buffer, sizeof(buffer), "%s - %-40s (%d mobuls, $%d)",
                 cp->code, cp->desc, cp->time, EDU_COST);
        pl_add(picker, n, 'a'+n, buffer);
    }
    ASSERT(count > 0);

    index = -1;
    pick_item(picker, heading, &index);

    pl_free(picker);

    return index;
}/* pick_course*/

