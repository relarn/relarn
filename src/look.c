// This file is part of ReLarn; Copyright (C) 1986 - 2018; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include "look.h"

#include "internal_assert.h"

#include "display.h"
#include "store.h"
#include "game.h"
#include "action.h"
#include "bank.h"

#include <unistd.h>

static void finditem(void);
static void olamp();
static void ogold();
static void fch(int how, long *x);
static void fntchange(int how);
static void oaltar(void);
static void othrone(void);
static void ochest(void);
static void ofountain(void);
static void obook(void);
static void opotion(void);
static void ostairs(void);
static void oscroll(void);
static void opit(void);
static void obottomless(void);
static void oelevator(void);
static void ocookie(void);
static void ohome(void);
static void oannihilation();
static void osimple(void);
static void oteleport_trap(void);
static void oshop(void);
static void ocoke(void);
static void oshrooms(void);
static void oacid(void);
static void ohash(void);
static void ospeed(void);
static void ocloseddoor(void);
static void oentrance(void);
static void ovoldown(void);
static void ovolup(void);
static void opointytraps(void);
static void otrapdoor();
static void take_stairs(int);
static void ocaveexit(void);


/* Look for an object and give the player options if an object was
 * found. Returns true if an object was found, false if not.  */
bool
lookforobject() {
    int i;
    struct Object thing;

    /* can't find objects is time is stopped*/
    if (UU.timestop) return false;

    thing = Map[UU.x][UU.y].obj;
    if (isnone(thing)) return false;

    showplayerarea();

    if (ispotion(thing)) {
        opotion();
        return true;
    }/* if */

    if (isscroll(thing)) {
        oscroll();
        return true;
    }

    i = thing.type;
    switch(i) {
    case OGOLDPILE: 
        ogold();
        break;

    case OALTAR:    
        oaltar(); 
        break;

    case OBOOK: 
        obook(); 
        break;

    case OCOOKIE:   
        ocookie(); 
        break;

    case OTHRONE:   
    case OTHRONE2:  
    case ODEADTHRONE: 
        othrone(); 
        break;

    case OBRASSLAMP: 
        olamp();
        break;

    case OPIT:  
        opit(); 
        break;

    case OSTAIRSUP: 
    case OSTAIRSDOWN:
        ostairs();
        break;

    case OELEVATORUP:
    case OELEVATORDOWN: 
        oelevator();
        break;

    case OFOUNTAIN: 
        ofountain(); 
        break;

    case OMIRROR:
    case OSTATUE:
    case OOPENDOOR:
    case ODEADFOUNTAIN:
        osimple();
        break;

    case OCHEST:    
        ochest();  
        break;

    case OIVTELETRAP:  
    case OTELEPORTER:
        oteleport_trap();
        break; 

    case OSCHOOL:
    case OBANK2:
    case OBANK:
    case OTRADEPOST:
    case OHOME: 
    case OPAD:  
    case ODNDSTORE: 
    case OLRS:  
        oshop();
        break;

    case OCLOSEDDOOR:
        ocloseddoor();
        break;

    case OENTRANCE: 
        oentrance();
        break;

    case OEXIT: 
        ocaveexit();
        break;

    case OVOLDOWN:
        ovoldown();
        break;

    case OVOLUP:
        ovolup();
        break;

    case OTRAPARROWIV:  
    case OTRAPARROW:
    case OIVDARTRAP:    
    case ODARTRAP:      
        opointytraps();
        break;

    case OIVTRAPDOOR:
    case OTRAPDOOR:
        otrapdoor();
        break;

    case OSPEED:    
        ospeed();
        break;

    case OSHROOMS:  
        oshrooms();
        break;

    case OACID: 
        oacid();
        break;

    case OHASH: 
        ohash();
        break;

    case OCOKE: 
        ocoke();
        break;  

    case OWALL: 
        break;

    case OANNIHILATION:
        oannihilation();
        break;

    case OWWAND:
    case OHANDofFEAR:
    case OORB:  
    default:    
        finditem(); 
        break;
    }/* switch*/

    return true;
}/* lookforobject*/



static void
otrapdoor() {
    bool invisible;

    invisible = Map[UU.x][UU.y].obj.type == OIVTRAPDOOR;

    if (invisible) {
        if (rnd(17) < 13) return;
        Map[UU.x][UU.y].obj = obj(OTRAPDOOR, 0);
        Map[UU.x][UU.y].know = 1;
    }/* if */

    if (has_a(OWWAND)) {
        say("You escape a trap door.\n");
        return;
    }/* if */

    if (getlevel() == DBOTTOM || getlevel() == VBOTTOM) {
        say("You fall through a trap door leading straight to HELL!\n");
        headsup();
        nap(3000);
        game_over_probably(DDTRAPDOORHELL); 
        return; /* Not reached. */
    }/* if */

    say("You fall through a trap door!\n");
    headsup(); 
    losehp(rnd(5+getlevel()), DDTRAPDOOR);
    nap(2000);
    setlevel(getlevel()+1);  

    update_display(true);
}/* otrapdoor*/



static void
opointytraps() {
    struct Object *atThing;
    bool invisible, isDart;
    const char *arrow;

    atThing = &Map[UU.x][UU.y].obj;

    invisible = atThing->type == OTRAPARROWIV || atThing->type == OIVDARTRAP;
    isDart = atThing->type == OIVDARTRAP || atThing->type == ODARTRAP;
    arrow = isDart ? "dart" : "arrow";

    /* If the trap is undiscovered, roll to see if it was tripped. */
    if (invisible) {
        if (rnd(17) < 13) return;
        Map[UU.x][UU.y].obj = obj(isDart ? ODARTRAP : OTRAPARROW, 0);
        Map[UU.x][UU.y].know = 0;
    }/* if */

    say("You are hit by %s %s!\n", an(arrow), arrow);
    headsup(); /* grrrr */

    if (isDart) {
        losehp(rnd(5), DDDART);
        // TODO: distinguish between base and total
        if ((--UU.strength) < 3) UU.strength = 3;
    } else {
        losehp(rnd(10)+getlevel(), DDARROW);
    }/* if .. else*/
}/* pointytraps*/


static void
ovoldown() {
    char opt;

    say("You have found %s\n", Types[OVOLDOWN].desc);
    opt = prompt ("Do you (c) climb down, or (i) ignore it? "); 
    if (opt != 'c') { 
        ignore();  
        return;
    }/* if */

    if (getlevel() != 0) { 
        say("The shaft only extends 5 feet downward!\n"); 
        return;
    }/* if */

    if (packweight() > 45+3*(UU.strength+UU.strextra)) { 
        say("You slip and fall down the shaft.\n");
        headsup();
        losehp(30+rnd(20), DDSHAFT); 
    } else {
        say("climb down.");
    }/* if .. else*/

    nap(3000); 
    setlevel(VTOP); /* down to V1 */
    UU.x = rnd(MAXX-2);
    UU.y = rnd(MAXY-2);
    positionplayer(); 

    update_display(true);
}/* ovoldown*/


static void
ovolup() {
    char opt;

    say("You have found %s\n", Types[OVOLUP].desc);

    opt = prompt("Do you (c) climb up, or (i) ignore it?"); 
    if (opt != 'c') {
        ignore();
        return;
    }/* if */

    if (packweight() > 40+5*(UU.dexterity+UU.strength+UU.strextra)) { 
        say("You slip and fall down the shaft.\n");
        headsup();
        losehp(15+rnd(20), DDSHAFT); 
        return;
    }/* if */

    say("climb up.\n");

    nap(3000); 
    setlevel(0);

    findobj(OVOLDOWN, &UU.x, &UU.y);

    update_display(true);
}/* ovolup*/


static void
oentrance() {
    char opt;

    say("You have found %s\n", Types[OENTRANCE].desc);
    opt = prompt("Do you (g) go inside, or (i) ignore it? ");
    if (opt != 'g') {
        say("ignore.\n");    
        return;
    }/* if */

    /* Move the player to level 1, creating it if necessary. */
    setlevel(1); 
    findobj(OEXIT, &UU.x, &UU.y);

    /* Update the display. */
    update_display(true); 
}/* oentrance*/


static void
ocaveexit() {
    char opt;

    say("You have found %s\n", Types[OEXIT].desc);
    opt = prompt("Do you (g) get out of here or (i) ignore it? ");
    if (opt != 'g') {
        say("ignore.\n");    
        return;
    }/* if */
    
    // TODO: look into using take_stairs() for this

    setlevel(0);
    findobj(OENTRANCE, &UU.x, &UU.y);
    update_display(true);
}/* ocaveexit*/

static void
ocloseddoor() {
    char opt;
    struct Object thing = Map[UU.x][UU.y].obj;

    say("You find %s. ", objname(thing));

    opt = prompt("Do you (o) try to open it, or (i) ignore it? ");
    if (opt == 'i' || opt == 0) { 
        say("ignore.\n");    
        moveplayer_back();
        return;
    }/* if */

    /* Attempt to open the door.  On failure, the player may encounter
     * a nasty surprise.  */
    say("open.\n");

    if (rnd(11)<7) {    // XXX invert this to remove the 'if'.

        switch(thing.iarg) {
        case DT_AGGRAVATE:
            say("The door handle squeaks loudly.\n");
            UU.aggravate += rnd(400);   
            break;
            
        case DT_SHOCK:
            say("You are jolted by an electric shock!\n");
            losehp(rnd(20), DDSHOCK);  
            break;
            
        case DT_WEAKEN: 
            say("You suddenly feel weaker!\n");
            if (UU.strength>3) UU.strength--;
            break;
            
        default:
            say("The door is stuck.\n");
            break;
        }
        moveplayer_back();
        return;
    }/* if */

    udelobj();  
    Map[UU.x][UU.y].obj = obj(OOPENDOOR, 0);
}/* ocloseddoor*/


static void
ospeed() {
    char opt;

    say("You find some speed.\n");
    opt = prompt("Do you (s) snort it, (t) take it, or (i) ignore it? ");

    if (opt == 's') {
        say("snort!\nOhwowmanlikethingstotallyseemtoslowdown!\n");

        UU.hasteSelf += 200 + UU.level;
        UU.halfdam += 300 + rnd(200);

        if ((UU.intelligence-=2) < 3)  UU.intelligence = 3;
        if ((UU.wisdom-=2) < 3)        UU.wisdom = 3;
        if ((UU.constitution-=2) < 3)  UU.constitution = 3;
        if ((UU.dexterity-=2) < 3)     UU.dexterity = 3;
        if ((UU.strength-=2) < 3)      UU.strength = 3;

        udelobj();
    } else if (opt == 't') {
        say("take.\n");
        pickup();
    } else {
        say("ignore.\n");
    }/* if .. else*/
}/* ospeed*/


static void
ohash() {
    char opt;

    say("You find some hashish.\n");
    opt = prompt("Do you (s) smoke it, (t) take it, or (i) ignore it? ");

    if (opt == 's') {
        say("smoke!\nWOW! You feel stooooooned...\n");
        UU.hastemonst += rnd(75)+25;
        UU.intelligence += 2;
        UU.wisdom += 2;
        if( (UU.constitution-=2) < 3) UU.constitution = 3;
        if( (UU.dexterity-=2) < 3)    UU.dexterity = 3;
        UU.halfdam += 300 + rnd(200);
        UU.clumsiness += rnd(1800) + 200;
        udelobj();
    } else if (opt == 't') {
        say("take.\n");
        pickup();
    } else {
        say("ignore.\n");
    }/* if .. else*/
}/* ohash*/


static void
oacid() {
    char opt;

    say("You find some LSD.\n");
    opt = prompt("Do you (e) eat it, (t) take it, or (i) ignore it? ");

    if (opt == 'e') {
        int j,k;

        say("eat!\n");
        say("You are now frying your ass off!\n");

        UU.confuse += 30 + rnd(10);
        UU.wisdom += 2;
        UU.intelligence += 2;
        UU.awareness += 1500;
        UU.aggravate += 1500;

        /* heal monsters */
        for(j=0;j<MAXY;j++) {
            for(k=0;k<MAXX;k++) {
                uint8_t monID = Map[k][j].mon.id;
                if (monID) {
                    Map[k][j].mon.hitp = mon_hp(monID);
                }/* if */
            }/* for*/
        }/* for*/

        udelobj();
    } else if (opt == 't') {
        say("take.\n");
        pickup();
    } else {
        say("ignore.\n");
    }/* if .. else*/
}/* oacid*/


/* Look! Shrooms! */
static void
oshrooms() {
    char opt;

    say("You find some magic mushrooms.\n");
    opt = prompt("Do you (e) eat them, (t) take them, or (i) ignore them? ");

    if (opt == 'e') {
        say("eat!\n");
        say("Things start to get real spacey...\n");
        UU.hastemonst += rnd(75) + 25;
        UU.confuse += 30+rnd(10);
        UU.wisdom+=2;
        UU.charisma+=2;
        udelobj();
    } else if (opt == 't') {
        say("take.\n");
        pickup();
    } else {
        say("ignore.\n");
    }/* if .. else*/
}/* shrooms*/



/* Cocaine is a helluva drug! */
static void
ocoke() {
    char opt;

    say("You find some cocaine.\n");
    opt = prompt("Do you want to (s) snort it, (t) take it, or (i) ignore it? ");

    if (opt == 's') {
        say("snort!\n");
        say("Your nose begins to bleed!\n");
        if ((UU.dexterity -= 2) < 3)     UU.dexterity=3;
        if ((UU.constitution -= 2) < 3)  UU.constitution=3;
        UU.charisma += 3;
        add_to_base_stats(33);
        UU.coked += 10;
        udelobj();
    } else if (opt == 't') {
        say("take.\n");
        pickup();
    } else {
        say("ignore.\n");
    }/* if .. else*/
}/* ocoke*/


/* Handle entering various shops and buildings. */
static void
oshop() {
    struct Object thing = Map[UU.x][UU.y].obj;
    char promptText[200];
    char opt;
    bool isPad = thing.type == OPAD;
    bool isHome = thing.type == OHOME;
    char *stay = isPad ? "check it out" : "go inside";
    char *go =   isPad ? "forget it." : "stay here.";

    /* Do nothing if the player is in combat.*/
    if (nearbymonst()) return;

    /* Ask the player if he/she wants to enter.  If not, quit. */
    snprintf(promptText, sizeof(promptText),
             "You have found %s.\nDo you (g) %s, or (i) stay here? ",
             isHome ? "your way home" : Types[thing.type].desc,
             stay);
    opt = prompt(promptText);
    if (opt == 'i' || opt == 0) {
        say("%s \n", go);
        return;
    }/* if */

    say("\n");

    switch (thing.type) {
    case OSCHOOL:       oschool();      break;
    case OBANK2:        obank(false);   break;
    case OBANK:         obank(true);    break;
    case OTRADEPOST:    otradepost();   break;
    case OHOME:         ohome();        break;
    case OPAD:          opad();         break;
    case ODNDSTORE:     dndstore();     break;
    case OLRS:          olrs();         break;
    default:
        FAIL("Unknown store type.");
    }/* switch */
}/* oshop*/




/*
  Function to handle simple objects.
*/
static void
osimple() {
    struct Object thing;
    const char *msg;

    if (nearbymonst()) return;

    /* Yes, this repeats code in the caller. */
    thing = Map[UU.x][UU.y].obj;
    
    switch (thing.type) {
    case OSTATUE:   
        msg = "You stand before a statue.";
        break;

    case OMIRROR:
        msg ="There is a mirror here.";
        break;     
        
    case ODEADFOUNTAIN: 
        msg = "There is a dead fountain here.";
        break;

    case OOPENDOOR:
        msg = "There is an open door here.";
        break;

    default:
        FAIL("Unexpected object found for osimple()");
    }/* switch */

    say("%s\n", msg);
}/* osimple*/


static void
oteleport_trap() {
    struct Object *atThing;

    atThing = &Map[UU.x][UU.y].obj;
    
    /* If it's a hidden (unknown) trap, first reveal it (unless the
     * player hasn't set it off. */
    if (atThing->type == OIVTELETRAP) {
        if (rnd(11)<6) return;  /* If it wasn't set off... */
        Map[UU.x][UU.y].obj.type = OTELEPORTER;
        Map[UU.x][UU.y].know = true;
    }/* if */

    say("Zaaaappp!  You've been teleported!\n");
    headsup(); 
    nap(3000); 
    teleport(0);
}/* oteleport_trap*/


/*
  function to say what object we found and ask if player wants to take it
*/
static void 
finditem() {
    int i;
    struct Object itm;
    char attrib[40] = "";

    if (nearbymonst()) return;

    itm = Map[UU.x][UU.y].obj;

    if (Types[itm.type].flags & (OA_WEARABLE|OA_WIELDABLE) && itm.iarg > 0) {
        snprintf(attrib, sizeof(attrib), " +%u", (unsigned) itm.iarg);
    }/* if */

    say("You find %s%s.\n", objname(itm), attrib);

    i = prompt("Do you want to (t) take it, or (i) ignore it?"); 
    if (i == 't') { 
        say("take.\n");
        pickup();
        return; 
    }

    ignore();
}/* finditem*/


/* Switch to the next/previous level.  Distance is added to current
 * level and should be -1 or 1.  */
static void
take_stairs(int distance) {
    int level = getlevel();
    enum OBJECT_ID entryType;

    /* Skip if there should be no stairs up.  (DBOTTOM+1 is the top
     * level of the Volcano and can only be exited via the volcano
     * shaft.  The top dungeon level doesn't have stairs up
     * either.) */
    if (distance < 0 && (level < 2 || level == DBOTTOM+1)) return;

    /* Also skip it if we're at the bottom of a dungeon level. */
    if (distance > 0 && (level == DBOTTOM || level == VBOTTOM)) return;

    /* Find the matching stairs and place the player there. */
    entryType = (distance > 0) ? OSTAIRSUP : OSTAIRSDOWN;
    setlevel(level + distance);
    findobj(entryType, &UU.x, &UU.y);

    /* Update the display. */
    update_display(true);
}/* take_stairs*/


// Function to handle staircases, both up and down.
static void
ostairs() {
    struct Object obj = Map[UU.x][UU.y].obj;
    const char *what;

    ASSERT(obj.type == OSTAIRSUP || obj.type == OSTAIRSDOWN);

    say("There is a circular staircase here.\n"); 

    what = (obj.type == OSTAIRSUP) ?
        "Do you (i) ignore it or (u) go up?"  :
        "Do you (i) ignore it or (d) go down?";

    switch(prompt(what)) {
    case 0:
    case 'i':   
        say("stay here.\n");   
        break;

    case 'u':
        say("go up.\n");
        ASSERT(obj.type == OSTAIRSUP);
        take_stairs(-1);
        break;

    case 'd':   
        say("go down.\n");
        ASSERT(obj.type == OSTAIRSDOWN);
        take_stairs(1);
        break;
    }/* switch*/
}/* ostairs*/


static void
lamp_spell() {
    int spell_id;

    say("A magic genie appears!\n");

    spell_id = pickspell(PM_LEARN);
    if (spell_id == -1) {
        say("The genie shrugs.\n");
        return;
    }/* if */

    ASSERT(spell_id >= 0 && spell_id < SPNUM);

    GS.spellknow[spell_id] = true;
    say("Spell \"%s\": %s\n%s\n", Spells[spell_id].code,
        Spells[spell_id].name, Spells[spell_id].desc);
}/* lamp_spell*/



static void
olamp() {
    char key;
    int roll;

    /* Query the user for actions.  If (s)he doesn't rub the lamp, we
     * return. */
    say("You find a brass lamp.\n");
    
    key = prompt("Do you want to (r) rub it, (t) take it, or (i) ignore it?");
    if (key == 'i' || key == 0) return;
    if (key == 't') {
        say("take.\n");
        pickup();
        return;
    }/* if */

    /* Otherwise, key == 'r' */

    /* See how irritable the genie is. */
    roll = rnd(100);
    if (roll > 90) {
        say("The magic genie was very upset at being disturbed!\n");
        losehp((int)UU.hp/2+1, DDGENIE);
        headsup();
    } /* higher level, better chance of spell */
    else if ( (rnd(100) + UU.level/2) > 80) {

        lamp_spell();

        say("The genie prefers not to be disturbed again.\n");
        udelobj();
        return;
    }
    else {
        say("nothing happened.\n");
    }

    if (rnd(100) < 15) {
        say("The genie prefers not to be disturbed again!\n");
        udelobj();
        UU.created[OBRASSLAMP] = false;  /* chance of finding lamp again */
    }
}/* olamp*/

void
ocookie() {
    say("\nYou find a fortune cookie.\n");
    switch(prompt("Do you (e) eat it, (t) take it, or (i) ignore it? ")) {
    case 0:
    case 'i':   
        ignore();   
        return;

    case 'e':   
        say("eat.\n");
        show_cookie();
        udelobj(); /* no more cookie */
        return;
            
    case 't':   
        say("take.\n");
        pickup();
        return;
    }/* switch*/
}/* ocookie*/


/* routine to pick up some gold -- if arg == OMAXGOLD then the pile is
 * worth 100* the argument */
static void
ogold() {
    long i;

    ASSERT (Map[UU.x][UU.y].obj.type == OGOLDPILE);

    i = Map[UU.x][UU.y].obj.iarg;
    say("You find %d gold piece%s.\n",i, i==1 ? "": "s");
    UU.gold += i;  

    Map[UU.x][UU.y].obj = NullObj;
    Map[UU.x][UU.y].know = 0;
}/* ogold*/



static void
ohome() {
    const char *congrats = 
        "        Congratulations.  You found the potion of cure\n"
        "        dianthroritis! Frankly, no one thought you could do it.\n"
        "\n"
        "        Boy!  Did you surprise them!"
        "\n";

    const char *however = 
        "\n"
        "        However...\n"
        "\n";

    const char *toolate = 
        "        The doctor has the sad duty to inform you that your daughter\n"
        "        has died! You didn't make it in time.  In your agony, you\n"
        "        kill the doctor, your %s and yourself!  Too bad...";

    const char *hooray =
        "        The doctor is now administering the potion and, in a few\n"
        "        moments, your daughter should be well on her way to\n"
        "        recovery.///\n"
        "\n"
        "        The potion is./././ working!\n"
        "\n"
        "        The doctor thinks that your daughter will recover in\n"
        "        a few days.  Congratulations!";

    const char *stillsick =
        "        Welcome home %s.\n"
        "\n"
        "        The latest word from the doctor is not good.  The\n"
        "        diagnosis is confirmed as dianthroritis.  He guesses\n"
        "        that your daughter has only %d mobuls left in this\n"
        "        world.\n"
        "\n"
        "        It's up to you, %s, to find the only hope for your\n"
        "        daughter, the very rare potion of cure dianthroritis.\n"
        "        It is rumored that only deep in the depths of the\n"
        "        caves can this potion be found.";

    char buffer[500];
    struct TextBuffer *msg;
    int cause = -1; /* -1 means not dead here. */
    char *spouse;

    spouse = UU.sex == MALE ? "wife" : "husband";

    // TO DO: add a delay code to showpages for the dramatic '...'
    msg = tb_malloc(INF_BUFFER, 79);

    tb_appendline(msg, "\n\n\n");

    if (has_a(OPCUREDIANTH)) {
        tb_appendline(msg, congrats);
        if (UU.gtime > TIMELIMIT) {
            tb_appendline(msg, however);
            
            snprintf(buffer, sizeof(buffer), toolate, spouse);
            tb_appendline(msg, buffer);
            cause = DDTOOLATE;
        } else {
            tb_appendline(msg, hooray);
            cause = DDWINNER;
        }
    } else if (UU.gtime > TIMELIMIT) {
        snprintf(buffer, sizeof(buffer), toolate, spouse);
        tb_appendline(msg, buffer);
        cause = DDTOOLATE;
    } else {
        snprintf(buffer, sizeof(buffer), stillsick, UU.name, 
                 (int)((TIMELIMIT-UU.gtime+99)/100),
                 UU.name);
        tb_appendline(msg, buffer);
    }/* if .. else*/

    showpages(msg);

    tb_free(msg);

    if (cause > 0) {
        game_over_probably(cause);
    }/* if */
}/* ohome*/


/*
function to process a potion
*/
static void
opotion() {
    struct Object pot;
    pot = Map[UU.x][UU.y].obj;

    ASSERT(ispotion(pot));

    say("You find %s. ", knownobjname(pot));

    switch(prompt("Do you (q) quaff it, (t) take it, or (i) ignore it?")) {
    case 0:
    case 'i':   
        ignore();  
        return;

    case 'q':   
        say("drink.\n");
        udelobj();   /*  destroy potion  */
        quaffpotion(pot);       
        return;

    case 't':   
        say("take.\n");
        pickup();
        return;
    }
}/* opotion*/


static void
raiserandom() {
    int stat = rund(6);

    switch (stat) {
    case 0: ++UU.strength; break;
    case 1: ++UU.intelligence; break;
    case 2: ++UU.wisdom; break;
    case 3: ++UU.constitution; break;
    case 4: ++UU.dexterity; break;
    case 5: ++UU.charisma; break;
    }/* switch */
}/* raiserandom*/


/*
  function to drink a potion
*/
void
quaffpotion(struct Object pot) {
    int i,j;
    int k;

    ASSERT (ispotion(pot));

    Types[pot.type].isKnown = true;  /* We know what it is *now* */

    say("You drink %s.\n", objname(pot));

    switch(pot.type) {
    case OPSLEEP:
        say("You fall asleep...\n");
        i=rnd(11)-(UU.constitution>>2)+2;
        while(--i>0) {
            onemove(DIR_STAY, false);
            nap(1000);
        }
        say(".. you wake up.\n");
        return;

    case OPHEALING:
        say("You feel better.\n");
        if (UU.hp == UU.hpmax)
            raisemhp(1);
        else if ((UU.hp += rnd(20)+20+UU.level) > UU.hpmax)
            UU.hp=UU.hpmax;
        break;

    case OPRAISELEVEL:
        say("You feel much more skillful!\n");
        raiselevel();
        raisemhp(1);
        return;

    case OPINCABILITY:
        say("You feel strange for a moment.\n");
        raiserandom();
        break;

    case OPWISDOM:
        say("You feel more self-confident!\n");
        UU.wisdom += rnd(2);
        break;

    case OPSTRENGTH:
        say("Wow!  You feel great!");
        if (UU.strength<12) UU.strength=12;
        else UU.strength++;
        break;

    case OPCHARISMA:
        say("Aaaoooww!  You're looking good now!\n");  
        UU.charisma++;
        break;

    case OPDIZZINESS:
        say("You become dizzy!\n");
        if (--UU.strength < 3) UU.strength=3;
        break;

    case OPLEARNING:
        say("You feel clever!\n");
        UU.intelligence++;
        break;

    case OPGOLDDET:
        say("You feel greedy...\n");
        nap(2000);
        for (i=0; i<MAXY; i++) {
            for (j=0; j<MAXX; j++) {
                if (Map[j][i].obj.type == OGOLDPILE) {
                    Map[j][i].know=1; 
                }
            }/* for */
        }/* for */
        update_display(true);
        return;

    case OPMONSTDET:
        for (i=0; i<MAXY; i++) {
            for (j=0; j<MAXX; j++) {
                if (Map[j][i].mon.id) {
                    Map[j][i].know=1; 
                }/* if */
            }/* for */
        }/* for */
        update_display(true);
        return;

    case OPFORGETFUL:
        say("You stagger for a moment...\n");
        for (i=0; i<MAXY; i++)  for (j=0; j<MAXX; j++)
            Map[j][i].know=0;
        nap(2000);  
        update_display(true);
        return;

    case OPWATER:
        say("This tastes bland.\n");
        return;

    case OPBLINDNESS:
        say("You can't see anything!\n"); 
        UU.blindCount+=500;  /* dang, that's a long time. */
        update_display(true);
        return;

    case OPCONFUSION:
        say("You feel confused.\n");
        UU.confuse+= 20+rnd(9); 
        return;

    case OPHEROISM:
        say("WOW!  You feel fantastic!\n");
        if (UU.hero==0) {
            add_to_base_stats(11);
        }/* if */
        UU.hero += 250;  
        break;

    case OPSTURDINESS:
        say("You feel healthier!\n");
        UU.constitution++;  
        break;

    case OPGIANTSTR:
        say("You now have incredible bulging muscles!\n");
        if (UU.giantstr==0) UU.strextra += 21;
        UU.giantstr += 700;  
        break;

    case OPFIRERESIST:
        say("You feel a chill run up your spine!\n");
        UU.fireresistance += 1000;  
        break;

    case OPTREASURE:
        say("You feel greedy...\n");
        nap(2000);
        for (i=0; i<MAXY; i++) {
            for (j=0; j<MAXX; j++) {
                k=Map[j][i].obj.type;
                if ((k==ODIAMOND) || (k==ORUBY) || (k==OEMERALD) 
                    || (k==OSAPPHIRE) || (k==OLARNEYE) 
                    || (k==OGOLDPILE)) {
                    Map[j][i].know = true; 
                    show1cell(j,i);
                }/* if */
            }/* for */
        }/* for */
        update_display(true);
        return;

    case OPINSTHEAL:
        UU.hp = UU.hpmax; 
        removecurse();
        break;

    case OPCUREDIANTH:
        say("You don't seem to be affected.\n");
        return;

    case OPPOISON:
        say("You feel a sickness engulf you!\n");
        UU.halfdam += 200 + rnd(200);  
        return;

    case OPSEEINVIS:
        say("You feel your vision sharpen.\n");
        UU.seeinvisible += rnd(1000)+400;
        MonType[INVISIBLESTALKER].mapchar = 'I';  
        return;
    };

    return;
}/* quaffpotion*/

// function to process a magic scroll
static void
oscroll() {
    struct Object scr = Map[UU.x][UU.y].obj;
    const char *prmsg;

    ASSERT (isscroll(scr));

    say("You find %s.\n", knownobjname(scr));

    prmsg = UU.blindCount == 0 ?
        "Do you (r) read it, (t) take it, or (i) ignore it?" :
        "Do you (t) take it or (i) ignore it?";

    switch(prompt(prmsg)) {
    case 0:
    case 'i':
        ignore();
        break;

    case 'r':   
        say("read.\n");
        udelobj();            /*  destroy it  */ 
        read_scroll(scr);  
        break;

    case 't':   
        say("take.\n");
        pickup();
        break;
    }/* switch*/
}/* oscroll*/


// You have been heard!  Get +3 protection.
static void
ohear() {
    say("You have been heard!\n");
    if (UU.altpro==0) 
        UU.moredefenses+=5;
    UU.altpro += 800;   /* protection field */
}/* ohear*/

static void
oaltar() {
    char *pmsg = "There is a holy altar here. \nDo you (p) pray, "
        "(m) donate money, (d) desecrate, or (i) ignore it?\n";

    if (nearbymonst()) return;

    switch(prompt(pmsg)) {
    case 'p': {
        int p;
        p = rund(100);
        if      (p < 12) createmonster(makemonst(getlevel()+2));
        else if (p < 17) enchweapon(ENCH_ALTAR);
        else if (p < 22) enchantarmor(ENCH_ALTAR);
        else if (p < 27) ohear();
        else             say("Nothing happens.\n");
        
        return;
    }

    case 'm': {
        long amt;
        int p;
        
        for (;;) {
            amt = numPrompt("How much do you donate? ", 0, UU.gold);
            if (amt < 0) continue;

            if (amt <= UU.gold) break;
                
            say("You don't have that much!\n");
            nap(1001);
        }/* for */

        if (amt < (UU.gold/10) && rnd(60) < 30) {
            say("Cheapskate! The Gods are insulted by such a "
                   "tiny offering!\n");
            udelobj();
            createmonster(DEMONPRINCE);
            UU.aggravate += 1500;
            return;
        }/* if */

        UU.gold -= amt;
        if (amt < (UU.gold+amt)/10 || (amt < rnd(50))) {
            createmonster(makemonst(getlevel()+2));
            UU.aggravate += 500;
            return;
        }/* if */

        p = rund(16);
        if (p <  4) { 
            say("Thank you.\n");
        } else if (p <  6) { 
            enchantarmor(ENCH_ALTAR);
            enchantarmor(ENCH_ALTAR);
        } else if (p < 8) {
            enchweapon(ENCH_ALTAR);
            enchweapon(ENCH_ALTAR);
        } else {
            ohear();
        }/* if .. else*/

        return;
    }

    case 'd': 
        say(" desecrate\n");
        if (rnd(100)<60) { 
            createmonster(makemonst(getlevel()+3)+8); 
            UU.aggravate += 2500; 
        } else if(rnd(100)<5) {
            raiselevel();
        } else if (rnd(101)<30) {
            say("The altar crumbles into a pile of dust before your eyes.\n");
            udelobj();    /*remember to destroy the altar*/
        } else {
            say("Nothing happens.\n");
        }/* if .. else*/
        return;
        
    case 'i':
    case 0: 
        ignore();
        if (rnd(100)<30) { 
            createmonster(makemonst(getlevel()+2)); 
            UU.aggravate += rnd(450); 
        }
        return;
    }/* switch*/

} /* end oaltar */


static void
othrone() {
    int roll, teleMin;
    struct Object throne;
    bool gnome, deadthrone;
    const char *prmsg;

    if (nearbymonst()) return;

    roll       = rnd(101);
    throne     = Map[UU.x][UU.y].obj;
    gnome      = (throne.type == OTHRONE);
    deadthrone = (throne.type == ODEADTHRONE);
    prmsg      = deadthrone ?
        "Do you (s) sit down, or (i) ignore it?" :
        "Do you (p) pry off jewels, (s) sit down, or (i) ignore it?";
    teleMin    = deadthrone ? 25 : 35;

    say("There is %s here.\n", objname(throne));

    switch(prompt(prmsg)) {
    case 0:
    case 'i':
        ignore();
        break;

    case 'p':   
        say(" pry off\n");  
        if (roll < 25) {
            int i;
            for (i=0; i<rnd(4); i++) {
                creategem(); /*gems pop off the throne*/
            }
            Map[UU.x][UU.y].obj = obj(ODEADTHRONE, 0);
            Map[UU.x][UU.y].know=0;
        }
        else if (gnome && roll < 40) {
            createmonster(GNOMEKING);
            Map[UU.x][UU.y].obj = obj(OTHRONE2, 0);
            Map[UU.x][UU.y].know=0;
        }
        else {
            say("Nothing happens.\n");
        }
        break;
        
    case 's':   
        say("sit down\n");  
        if (deadthrone && roll < 5) {
            raiselevel();
        }
        else if (gnome && roll < 30) {
            createmonster(GNOMEKING);
            Map[UU.x][UU.y].obj = obj(OTHRONE2, 0);
            Map[UU.x][UU.y].know=0;
        }
        else if (roll < teleMin) {
            say("Zaaaappp!  You've been teleported!\n\n"); 
            headsup();
            teleport(0); 
        }
        else {
            say("Nothing happens.\n");
        }
        return;
    }/* switch*/
}/* othrone*/



static void
ochest() {
    int i,k;
    char opt;

    say("There is a chest here.\n");  
    opt = prompt("Do you (t) take it, (o) try to open it, or "
                 "(i) ignore it? ");

    switch(opt) {
    case 'o':   
        say(" open it.\n");  
        k=rnd(101);
        if (k<40) {
            say("The chest explodes as you open it.\n"); 
            headsup();
            i = rnd(10);
            if (i > UU.hp) i = UU.hp;
            say("You suffer %d hit point%s damage!\n", (long)i,
                    i==1?"":"s");
            losehp(i, DDCHEST);
            switch(rnd(10)) {
            case 1: 
                UU.itching+= rnd(1000)+100;
                say("You feel an irritation spread over your skin!\n");
                headsup();
                break;
                
            case 2: 
                UU.clumsiness+= rnd(1600)+200;
                say("You begin to lose hand-eye co-ordination!\n");
                headsup();
                break;
                
            case 3: 
                UU.halfdam+= rnd(1600)+200;
                say("You suddenly feel sick and BARF all over your "
                       "shoes!\n");
                headsup();
                break;
            };
            Map[UU.x][UU.y].obj = NullObj;
            Map[UU.x][UU.y].know=0;
            if (rnd(100)<69) 
                creategem(); /* gems from the chest */
            dropgold(rnd(110*Map[UU.x][UU.y].obj.iarg+200));
            for (i=0; i<rnd(4); i++) 
                something(UU.x, UU.y, Map[UU.x][UU.y].obj.iarg+2);
        }
        else say("Nothing happens.\n");
        return;
        
    case 't':   
        say(" take\n");
        pickup();
        return;
        
    case 'i':
    case 0: 
        ignore(); 
        return;
    }/* switch*/
}/* ochest*/



static void
ofountain() {
    int x;

    if (nearbymonst()) return;
    say("There is a fountain here.\n"); 

    switch(prompt("Do you (d) drink, (w) wash yourself, or (i) ignore it?")) { 
    case 'i':   
    case 0:
        ignore();  
        break;

    case 'd':   
        say("drink\n");
        if (rnd(1501)<4) {
            say("Oops! You caught the dreadful sleep!\n");
            headsup();
            sleep(3);  
            game_over_probably(DDSLEEP); 
            return;
        }
        x = rnd(100);
        if (x==1) {
            raiselevel();
        } else if (x < 11) {  
            x=rnd((getlevel()<<2)+2);
            say("Bleah! The water tasted like stale gatorade! "
                    "You lose %d hit point%s!\n", (long)x, x==1?"":"s");
            losehp(x, DDBADWATER); 
        } else if (x<14) {    
            UU.halfdam += 200+rnd(200);
            say("The water makes you vomit.\n");
        } else if (x<17) {
            quaffpotion(obj(OPGIANTSTR, 0)); /* giant strength */
        } else if (x < 45) {
            say("Nothing seems to have happened.\n");
        } else if (rnd(3) != 2) {
            fntchange(1);   /*change char levels upward*/
        } else {
            fntchange(-1);  /*change char levels downward*/
        }/* if .. else*/

        if (rnd(12)<3) {      
            say("The fountains bubbling slowly quietens.\n");
            /* dead fountain */
            Map[UU.x][UU.y].obj = obj(ODEADFOUNTAIN, 0); 
            Map[UU.x][UU.y].know=0;
        }
        break;

    case 'w':   
        say("wash yourself.\n");
        if (rnd(100) < 11) {    
            x=rnd((getlevel()<<2)+2);
            say("The water burns like acid!  You lose %d hit point%s!\n",
                    (long)x, x==1?"":"s");
            losehp(x, DDBADWATER); 
        } else if (rnd(100) < 29) {
            say("You are now clean.\n");
        } else if (rnd(100) < 31) {
            say("This water needs soap -- the dirt didn't come off.\n");
        } else if (rnd(100) < 34) {
            createmonster(WATERLORD); 
        } else {
            say("Nothing seems to have happened.\n");
        }/* if .. else*/
        break;
    }/* prompt*/
}/* ofountain*/



/*
  Fountain effect: raise or lower character levels.

  If how > 0 they are raised; otherwise, they are lowered
*/
static void
fntchange(int how) {
    long j;

    switch(rnd(9)) {
    case 1: 
        say("Your strength");    
        fch(how,&UU.strength);  
        break;
    case 2: 
        say("Your intelligence");    
        fch(how,&UU.intelligence);  
        break;
    case 3: 
        say("Your wisdom");
        fch(how,&UU.wisdom);    
        break;
    case 4: 
        say("Your constitution");
        fch(how,&UU.constitution);  
        break;
    case 5: 
        say("Your dexterity");
        fch(how,&UU.dexterity); 
        break;
    case 6: 
        say("Your charm");
        fch(how,&UU.charisma);  
        break;
    case 7: 
        j=rnd(getlevel()+1);
        if (how < 0) {  
            say("You lose %d hit point%s!\n", (long)j, j==1?"":"s");
            losemhp((int)j);
        }
        else {  
            say("You gain %d hit point%s!\n",(long)j, j==1?"":"s");
            raisemhp((int)j);
        }
        break;
    case 8: 
        j=rnd(getlevel()+1);
        if (how > 0) {  
            say("You just gained %d spell%s!\n",(long)j, j==1?"":"s");
            raisemspells((int)j);
        }
        else {  
            say("You just lost %d spell%s!\n",(long)j, j==1?"":"s");
            losemspells((int)j);
        }
        break;
    case 9: 
        j = 5*rnd((getlevel()+1)*(getlevel()+1));
        if (how < 0) {  
            say("You just lost %d experience point%s!\n",(long)j,
                    j==1?"":"s");
            loseexperience((long)j);
        }
        else {  
            say("You just gained %d experience point%s!\n",(long)j,
                    j==1?"":"s");
            raiseexperience((long)j);
        }
        break;
    }/* switch*/
}/* fntchange*/

/*
 *  process an up/down of a character attribute for ofountain
 */
static void
fch(int how, long *x) {
    if (how < 0 )    {  
        if (*x > 3) {
            say(" went down by one!\n");   
            --(*x); 
        } else
            say(" remained unchanged!\n");
    } 
    else {
        say(" went up by one!\n"); 
        (*x)++; 
    }
}/* fch*/


// You've stepped on a Sphere of Annihilation.  Ooops.
static void
oannihilation() {
    if (has_a(OANNIHILATION)) {
        say("The Talisman of the Sphere protects you from "
               "annihilation!\n");
        return;
    }/* if */

    /* annihilated by sphere of annihilation */ 
    say("The sphere annihilates you!\n");
    nap(3000);
    game_over_probably(DDSPHERE);
}/* oannihilation*/


static void
opit() {
    int i;

    say("You're standing at the top of a pit.\n"); 

    if (rnd(101)>81) return;

    if (rnd(70) > 9*UU.dexterity-packweight() || rnd(101)<5) {
        if (has_a(OWWAND)) {
            say("You float right over the pit.\n");
            return;
        }

        if (getlevel()==DBOTTOM || getlevel() == VBOTTOM) {
            obottomless(); 
        } else {
            if (rnd(101)<20) {
                i=0;
                say("You fell ino a pit!\n"
                       "A poor monster cushions your fall!\n");
            } else {
                i = rnd(getlevel()*3+3);
                if (i > UU.hp) i = UU.hp;
                say("You fell into a pit!\n"
                        "You suffer %d hit point%s damage.\n",(long)i,
                        (i==1)?"":"s");
            }
            losehp(i, DDPIT);
            nap(2000);  
            setlevel(getlevel()+1);  
            update_display(true);
        }
    }/* if */
}/* opit*/


static void
obottomless() {
    say("You fell into a pit leading straight to HELL!\n");  
    headsup(); 
    nap(3000);  
    game_over_probably(DDHELL);
}/* obottomless*/


static void
oelevator() {
    struct Object obj = Map[UU.x][UU.y].obj;
    int level, newlevel;

    ASSERT(obj.type == OELEVATORUP || obj.type == OELEVATORDOWN);

    level = getlevel();
    say("You have found an express elevator going ");

    if (obj.type == OELEVATORUP) {
        say("up.\n");
        if (level == 0) {
            say(" Unfortunately, it is out of order\n.");
            return;
        }
        newlevel = rund(level);
    }
    else {
        say("down.\n");
        if (level==DBOTTOM || level==VBOTTOM) {
            nap(2000);
            say("It leads straight to HELL!!!!!!!!!\n");
            headsup();
            nap(3000);
            game_over_probably(DDHELLEVATOR);
        }
        newlevel = level + rnd(DBOTTOM - level);
    }/* if .. else*/

    UU.x = rnd(MAXX-2);  
    UU.y = rnd(MAXY-2);
    nap(2000);
    setlevel(newlevel);

    positionplayer();

    update_display(true);
}/* oelevator*/


static void
obook() {
    char ptext[120];
    snprintf (ptext, sizeof(ptext), "You find a book.\nDo you %s(t) take "
              "it, or (i) ignore it? ",
              UU.blindCount ? "" : "(r) read it, "); 

    switch(prompt(ptext)) {
    case 0:
    case 'i':   
        ignore();   
        break;

    case 'r':   
        say("read.\n");
        readbook(Map[UU.x][UU.y].obj.iarg);   
        udelobj();            /* no more book */
        break;

    case 't':   
        say("take.\n");
        pickup();
        break;
    }/* switch*/
}/* obook*/
