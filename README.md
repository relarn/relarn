
# ReLarn

ReLarn is a fork of the the classic Roguelike game Ularn that improves
the user interface and refactors the source code to be more
maintainable and better suited to the modern world.  The code has been
modularized with OS- and UI-dependencies abstracted away, all warnings
have been fixed, arrays replaced with structures and more.

The goal is to preserve the spirit of the original game on modern
computers.

See [Changes.md](Changes.md) for a more complete summary of changes.

* [Website](http://relarn.org)
* [Github](http://github.com/relarn/relarn)


## Getting Started

ReLarn should be easy to build on any sufficiently Unix-like system.
If it doesn't work out of the box, the build scripts are just small,
hand-written Makefiles and shell scripts so they're easy to fix.

It has been successfully built on Ubuntu Linux, Raspbian on a
Raspberry Pi 3, macOS with MacPorts, and Cygwin on Windows 7.


### Prerequisites

To build ReLarn, you will need:

* gcc, clang or some other C compiler that supports C99 and the common
  Unix compiler arguments.
* ncurses version 6 or better.  (Earlier versions may also work.)
* GNU Make
* `bash`
* Perl 5 (optional; `pod2html` is used to generate the man page)
* The common Unix utilities.

These should all be installed by default or easily available to your
package manager.  For example, on Ubuntu, the following command is
all you need:

    sudo apt-get install libncurses-dev gcc make perl

Note that the Makefile uses a bunch of Unixy compiler options so
(e.g.) Visual C won't work out of the box.  It may be possible to
hack up the make scripts until it works, but that's not going to be
easy.


### Configuration

For a program the size of ReLarn, automatic configuration systems
(e.g. CMake or autoconf) tend to be more trouble than they're worth;
instead, you configure ReLarn by editing the file `src/config.mk`.

Simply set `INST_ROOT` to the place you wish the game installed.

    INST_ROOT = /usr/local/games/

The default location is `~/apps/relarn`; if that's sufficient for you,
you don't need to do anything else.


### Building and Installing

Once configuration is done, simply do the standard `make` and `make
install` in `src/`:

    cd src
    make
    make install

And there you go.


### Building a Binary Release

You can also build a tar archive containing the playable game.  This
doesn't need `INST_ROOT` to be set at all.  Simply run make:

    cd src
    make
    make distbin

This will place the archive in the project root directory.


### When It Doesn't Work

If `make` fails with an error message like 

    config.mk:43: *** No settings for FooOS-bar_arch.  Stop.

you will need to add your platform to the list of supported OS-CPU
pairs in `config.mk`.  It's probably sufficient to copy one of the
Linux sections.

The Makefile sets the `-Werror` flag which makes all warnings
fatal.  This enforces better coding style during development but can
be a problem if you're just trying to play the stupid game; it is safe
to remove the flag, although you may want to confirm that the warning
isn't anything terrible.

If it's none of these, you'll need to debug it yourself.  Sorry.  Feel
free to contact me if you can't get it working or to submit a patch or
pull request if you do.


## Contributing

Pull requests are welcome.  In particular, I'm looking for bug fixes,
UI improvements and ports to new platforms.

Note that the goal of this project is to preserve the spirit of the
original game; as such, I'm not looking for new gameplay elements
(e.g. weapons, monsters, quests, etc., although stuff like balance
tweaks may be considered.)  If you're trying to create a different
game, you should fork this project and do it that way.


## Authors

See the file [AUTHORS.txt](AUTHORS.txt) for the list of known
contributors.  The Larn games have been hacked on for close to three
decades so it's impossible to track down everyone.

## License

This project is licensed under the GNU General Public License.  See
[Copyright.txt](Copyright.txt) and [LICENSE.txt](LICENSE.txt) for
details.

