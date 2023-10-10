# Change Log

Note that spoilery changes are listed in a following section if you
wish to avoid them.

## Changes in 2.2.2

Various bug fixes:
* Items with negative enchantment had a really high resale value.
* When selling gems at the bank, the price shown was the store price,
  not the bank price.
* The `-s` and `-i` had their meanings swapped.
(Thanks to Github user Meklon2007 for reporting these.)

Other bug fixes:
* Monsters were not setting off traps.
* The launch script will now work when launched via a symlink.

`-s` and `-i` now show an empty scoreboard if the score file is
missing or empty rather than just exiting.  They also show a message
if the score file appears to be corrupted.

The graphical build now includes a default font (Inconsolata Medium by
Raph Levien).

Simplified running relarn in a debugger by making the executable use
`../data` and `..` as the `lib` and `var` paths when run directly
(i.e. without the launch script) as you would from in the `src`
directory.  It should also no longer be necessary to edit any
make files for for ordinary out-of-the-box builds.

Other tidying.


## Changes in 2.2.0

Now supports native Microsoft Windows as a platform.  All Hail the
MinGW and MSYS teams.

Now builds as a native windowed application in addition to running on
a TTY (i.e. console).  All Hail the PDCurses and SDL teams. TTY mode is
still supported on non-Windows platforms.

Now lets you select colours for various elements in the relarnrc file.
In addition, there is now a nice set of default colours.  There is
also now a "dark-screen" option to improve colouring on light-on-dark
displays.

The town is now completely visible (it's daylight, after all) and
looks a bit more pleasant.

You can now scroll back on the message pane to see older messages.

Backspace now works as expected when entering numbers or other text.

When a displayed statistic changes (e.g. STR, Max HP), the stat is
temporarily displayed in bold or reverse-video to make it more
noticable.

The challenge level system is now deprecated.  The `-c` option is now
gone and challenge level is no longer displayed in the final message.
(It's still available in relarnrc and still affects your score.)

Added config option 'gui-email-client:'; like 'email-client:' but only
used by the SDL-based build.  This lets you specify a different email
client (or the same one but in an xterm) when there isn't a TTY
present.

Option 'no-introduction' is ignored if the user interactively created
a character.  In addition, the `-n` option has been removed.

Cleaned up the way the basic stats are managed.  This is a code
quality fix but may <strikeout>reduce cheating</strikeout>introduce
subtle changes in stat modification in some corner cases.

Successfully casting a spell will now always result in a descriptive
message.

The savefile ID is now less fragile.  Rebuilding on a system with an
updated kernel should no longer break savefile compatibility.

Renamed spell "Invulnerabilty" to "Invulnerable Globe".  This seems
implied by some variable names and makes for better lore.

Rewrote the Sphere of Annihilation code to make it simpler and to
simplify saving.

Fixed a bug where the Explorer's Guild Membership Certificate would
declare that the player had defeated **REDACTED** even when they
hadn't.

Generating the Diagnostic file (a debugging aid) no longer creates new
levels.

The ziller has now been renamed to its more common name, 'zill'.  (A
zill is a large carnivorous mustelid; its smaller cousin is known as
the 'wee zill'.)

The Genocide spell is now Banishment.  Mechanically, it's exactly the
same but magical banishment fits the lore better and is less
abhorrent.

Corrected the spelling of "jaculus".  (Was using the Latin plural
"jaculi".)

Fixed the formatting on some messages.

End-of-game email messags now put the current date in the `mbox`
separator; this was leading to incorrect dates on some email clients.

### Spoilery Changes

Permanence no longer applies to "Hold Monster".

Bug fix: the damage modifier from your current weapon is no longer
added to magical ranged attacks (e.g. magic missile).

Lemmings are now much less annoying after a few experience levels,
just enough to get the full Lemming Experience but not enough for it
to get old.

Destroying certain objects with Spheres of Annihilation will now have
more consequences: destroying Home loses the game immediately, the LRS
office adds $2000 to your outstanding tax bill and the University
revokes your degree (i.e. mark a course as not taken) and (attempts
to) delete your diploma if you've graduated.

The Diploma now makes you a little more money.  It will now not work
if you don't have all of your courses.  (This can happen if you drop
the diploma, destroy the University with a Sphere of Annihilation and
then pick it up again.)

Now smarter about keeping track of which level numbers the player
knows after a random level teleport, trap door or other events that
can send them to an unidentified level of unknown depth.  If the
player can deduce the depth of previously unknown levels, the game
will now reveal them.

Changed the way Create Monster finds a free spot to place the monster
so that it's a bit more random.

## Changes in 2.1

Broke savefile compatibility with 2.0.

Changed the semantics of vision and field-of-view:
* The previous version would (sometimes) display everything (including
  monsters) in any location that had previously been revealed.  Now,
  monsters are only visible within the field of view and everything else
  is just the map you made when you last saw it.
* The field of view is now computed based on line-of-sight (via Greg
  McIntyre's libfov) rather than just revealing the rectangle around the
  player; it is no longer possible to see through walls, for example.

The field of view is now (optionally) highlighted on terminals that
support colour.

Unexplored cells now look different from empty cells on colour TTYs.

The module `display.[ch]` now is the sole owner of field-of-view.

Added the `-b` debugging flag; don't use it.

Is now somewhat more efficient with bandwidth between program and
terminal.  (This may not actually be noticable to anyone not using an
acoustic modem but there's a small chance that it'll reduce someone's
network charges.)

You may now select your spouse's gender.

You may now select "non-binary" as a gender for yourself and/or your
spouse.

The menu options you get when stepping on an object now use (mostly)
the same keys in a consistent manner.

Standing on an object no longer pops up the associated menu for each
new turn.  Now, you only get the menu when you walk on it or if you
explicitly look (',').

Now more aggressive in finding a free spot when dropping items after
Create Artifact, Thrones, monster kills, etc.  The game used to give
up if there wasn't a free adjacent place; now, it looks farther away
and virtually guarantees the object will be placed on the map.


### Spoilery Changes

Enlightenment et. al. now work by increasing the visual radius and
letting you see through walls for a small number of turns.  This
changes the semantics a little.

Similarly, Potions of Monster Detection now make all monsters visible
for a few turns but do not reveal the map locations they occupy.

You can no longer identify walls or floor while blind if you can walk
through them.

Praying at an altar can now (very, very rarely) teleport you back to
the town with the Potion of Cure Dianthroritis, effectively winning
the game.  (True fact: this (instantly winning while praying) could
also happen in 2.00 as the result of a bug.)

Haste-self now decreases the passage of time by half.  (It used to
stop the counter completely; this meant that permanent haste-self
effectively ended the time limit.)

Loading a saved game no longer gives you a free regeneration turn.
(This was probably a bug, anyway.)

Using an object may now consume a turn if you don't do so when walking
on it.  This is because the action menu gets suppressed if you haven't
moved and bringing it up requires an explicit look (via ','), which
consumes a turn.

Made lemmings somewhat easier to kill. (I mean, it's a good feature
but it gets old fast.)

The spell "Time Stop" was not stopping the turn counter from
advancing.  It does now.

It is no longer possible to walk through a closed door when "Time
Stop" is in effect.


## Changes in 2.00 (i.e. from Ularn 1.5ish)

Here is a (hopefully) complete list of notable differences between
ReLarn and Ularn 1.5ish from which it was forked.


### Maintainability Fixes

The source code has been organized into thematically-related modules
consisting of two files, a source (`.c`) file and a matching header
(`.h`) where the header provides a public interface to the source
file.  You can tell if a module uses another module by whether or not
it includes the corresponding header somewhere.  In addition, a few
modules follow the OOP pattern, where an object is implemented using a
struct and a number of functions that take it as the first argument.

All functions now have C90-style prototypes, no exceptions.

The global player state was converted from an array of `long` to a
(global) struct and renamed from `c` to `UU`.

More globals were folded into a global structure named `GS` (for game
state).

Most remaining global variables are now gone, made `static` or folded
into either `struct Player` or `struct GameState`.

Most macros were replaced with inline functions.

Most `#define`s were replaced with `enum`s.

Removed many gotos.

Renamed lots of things to more descriptive names.

Merged lots of arrays into structs or arrays of structs.

Replaced lots of bare numbers with corresponding enums.

Eliminated all code that causes compiler warnings on GCC and Clang.

Where possible, operating system dependencies have been replaced with
functionality found in the standard library.  Where that hasn't been
possible, the functionality has been made as close to POSIX as
possible and moved to the module `os.c`.

In the same way, the user interface code has been moved to `ui.c`, and
the interface itself made independent of the current (textual)
implemetation. It should be straightforward to make a GUI-based
interface.

Spell immunities have been reorganized into a more maintainable data
structure.

Merged the map into a single matrix of structs; it had previously been
several different arrays of structs.

Large lists of related data have been consolidated into files of macro
invocations.  These can be turned into different types of C literals
by redefining the macro and then including the file.  For example,
`monster_list.h` is used to initialize the array-of-structs containing
the monster types and later to create the enum items that index it.

Most signal handling has now been removed.  In particular, emergency
autosaving on a `SIGINT` or `SIGTERM` only happens when it can be
guaranteed that the game state is coherent.  To make up for this,
`relarn` now regularly autosaves and keeps the previous save around.

Taxes are now computed once at the start of the game and stored in the
save file rather than retrieved from the savefile at startup.  This
*probably* isn't visible to the player.

Most comments were reviewed and updated.


## Modernization Fixes

Made non-value-returning functions void.

As much as possible, replaced all Unix I/O code with `stdio.h`
functions.

Replaced low-level TTY code with ncurses.

Replaced metaconfig with GNU Make and bash.  (Insert rant about how
autoconfig systems are more trouble than they're worth, especially
given the ubiquity of F/OSS Unix tools.)

Got rid of most of anti-cheating stuff.  Ularn was written for
multi-user Unix systems so it made sense to wall away the scorefile
behind permissions.  These days, people play it on their own computer
where cheating is pointless or log in to a remote server with
locked-down accounts.

Moved the copyright message to `Copyright.txt` and reworded it
slightly.  In particular, the license has switched to the GPL version
2, as allowed in the Ularn copyright statement.

Removed GNU getopt sources; it comes with your OS these days.

The score file is now textual and scores are appended.

Save files and per-user configuration files are now stored in the
directory `~/.relarnrc`.


## UI Changes

There is now a visual selector that gets used for most menus and
menu-like things.  This includes selecting inventory items, spells,
store items, arbitrary menus, etc.

Added a document viewer for displaying textual pop-ups (e.g. the
online help, the introduction text, the list of items to be sold,
etc.)

Debugging is now done differently; it is active only if the program
can find its own executable and has write access to it.  If this is
the case, the '_' key will bring up a menu of debug items.  The
original "wizard mode" feature is still present but can now be
toggled.

Monster renaming has been removed.

Junk mail is now handled differently.  Instead of sending you email
with `/usr/bin/mail`, ReLarn creates an `mbox`-format mailbox in
`~/.relarnrc`.  The user may configure an email client in which case,
the client will be launched on the folder when the game ends after the
player's win.



### Gameplay Changes

**Note that these could be considered spoilers, so you may want to skip
this section if that's a problem for you.**

The semantics of stolen items have changed.  Monsters that steal no
longer carry the item around.  Instead, it is immediately fenced and
sold to a thief on that level who may drop it when killed.

A Rambo can sell the Lance of Death now but will get full price for
it.

The bank slogan changes randomly after a few visits.

Renamed a few things.  The land is now Larn rather than Ularn; the
College is now a University.

Completing all courses at the University gives you an artifact, the
Diploma.  Carrying the Diploma increases the amount of gold monsters
drop.

Added the obligatory Jabberwocky reference to the Vorpal Blade.

Your final score is now more than just the gold you've accumulated.

Missile attacks against the LRS do damage that is added to your tax
bill.  (I stole this from iLarn.)

Added or changed a number of messages to make them more humorous.

The junk mail items you receive are now randomly chosen (with two
exceptions).  I also added two more possibilities.

Changed format of end-of-game (i.e you win or die) message.

Enlightenment (scroll and spell) now might last for a small number of
turns.
