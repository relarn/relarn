# Changes from Ularn 1.5ish

Here is a (hopefully) complete list of notable differences between
ReLarn and Ularn 1.5ish from which it was forked.


## Maintainability Fixes

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



## Gameplay Changes

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

