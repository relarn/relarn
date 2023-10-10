# Building ReLarn

ReLarn should be easy to build on any sufficiently Unix-like system.
If it doesn't work out of the box, the build scripts are just small,
hand-written Makefiles and shell scripts so they're easy to fix.

It has been successfully built on Ubuntu Linux, Raspbian on a
Raspberry Pi 3, macOS with MacPorts, and Cygwin on Windows 7.

The native Windows version requires MSYS and MINGW; I use
[MSYS2](https://www.msys2.org/) and gcc.

The macOS app uses only command-line tools, scripts and trickery.

## Requirements

To build basic ReLarn, you will need:

* gcc, clang or some other C compiler that supports C99 and the common
  Unix compiler arguments.
* ncurses version 6 or better.  (Earlier versions may also work.)
* GNU Make
* A sufficiently Unix-ish environment including `bash` and `perl`
  (optional; `pod2html` is used to generate the man page).  On
  Windows, MSYS is sufficient

To build the graphical version of ReLarn, you will also need

* PDCurses 3.8 (cloned [here](https://gitlab.com/relarn/relarn-pdcurses.git)
  for your convenience with some makefile tweaks) instead of ncurses.
* SDL2 and SDL2_ttf and all their dependencies


## Quick Start

Note that this builds the TTY (i.e. console-mode) version only.  The
graphical version (required for (e.g.) non-Cygwin Windows) is more
involved.  See below.

1. Install the requirements.  If you have trouble finding them, I may
   have some hints below.

2. Run make:
```sh
    cd src
    make tarball
```

3. Extract the resulting tarball somewhere:
```sh
    cd ~/my_games
    tar xvf <path-to-relarn>/src/relarn-*.tar.gz
```

4. Create a symlink from `bin/relarn` to somewhere in your path:
```sh
    ln -s ~/my_games/relarn-<version>/bin/relarn ~/my_games/bin
```
   (This is optional; you could also add the install directory to your
   path if you want.)

If you want to do the classic Unix thing and install it globally in a
system directory such as `/usr/local`, you can also do that:

```sh
    cd src
    make RELEASE=y INST_ROOT=/usr/local install
```

However, I've had better luck doing the tarball thing and just
extracting it somewhere globally accessible on the machine.


## Makefile configuration

The Makefile accepts three arguments:

* `INST_ROOT` is the place to install the game.  `tarball` works by
  setting it to a local directory that it then archives.
* `RELEASE` is a flag; when set to "y" (or any other non-empty
  string), compiles for release instead of development.
* `PDCURSES` is the path to an SDL-enabled PDCurses build.  If
  present, make will build the graphical version of relarn.

Invoking `make` with no arguments will produce a debug build in the
`src` directory.  This can be run in place for debugging.

The relevant `make` targets are:

* `make install` builds the game and installes at at `INST_ROOT`

* `make install_dist` is like `make install` but also installs binary
  dependencies with the expectation that you'll want to zip or tar the
  directory run it elsewhere.

* `make tarball` does `make install_dist` and produces a tar archive
  for you with a meaningful name.

* `make app` on macOS will (try to) build ReLarn.app

* `make all/clean/tags/et. al.` do the usual expected things.



## Building the Graphical Version

ReLarn can run on systems without a proper terminal emulator by using
PDCurses and its interface to SDL.  (That's right, PDCurses can work
on a framebuffer!)  We use the SDL 2.x target with TrueType font
support for this.

Building it is pretty straightforward:

First, clone the repo.  I use my copy for security but it's no
different from the upstream.

    git clone https://gitlab.com/relarn/relarn-pdcurses.git

Also install the SDL dependencies (SDL2 and the TrueType font
extension) if you haven't already.  On Ubuntu, this is:

    sudo apt install libsdl2-dev libsdl2-ttf-dev

The PDCurses source tree has a subdirectory for each backend target.
You just `cd` into the correct one and run make.  In our case, we also
need to specify wide characters with the option `WIDE=Y`:

    cd relarn-pdcurses/sdl2
    make WIDE=Y

Then, you simply need to build `relarn` with argument `PDCURSES`
pointing to the the checked-out PDCurses source tree:

    make PDCURSES=<path>/relarn-pdcurses tarball

That works pretty much universally but requires MSYS (or equivalent)
on Windows.





## Platform-specific Hints

### Ubuntu Linux

As of this writing (Ubuntu 20.04LTS), you can install all of the
necessary dependencies with:

    sudo apt-get install libncurses-dev gcc make perl \
        libsdl2-dev libsdl2-ttf-dev

(You can skip the last two if you only want to build the text-mode
game.)

Given the variance in Linux packaging systems, your mileage will vary
across versions and distros.  Then again, these are all really stable
packages so this or your distro's equivalent may well Just Work.


### macOS

macOS is Just A Unix and `make tarball` et. al. should just work as
expected.  I used Xcode's command-line tools to compile and link it
and got `sdl2` and `sdl2_ttf` from Brew.

Unlike on Linux, the tarballs will also include the necessary
`*.dylib` files needed to redistribute the game.  These are gathered
by the script `build_helpers/fetch_osx_dylibs.sh`, which may or may
not work correctly.  (Unfortunately, I macOS makes it difficult to
affordably set up clean build or test environments, so "works on my
machine" is the best I can offer.)

There is also (experimental!) support for producing a macOS App for
the graphical build, via the "app" target:

    make PDCURSES=<path>/relarn-pdcurses app

This will create a directory named `ReLarn.app` in `src/`.  As above,
all I can promise is that it works on my machine.


### Windows

Note that the Windows port is semi-abandoned for now.  I almost never
use Windows anymore and don't have a development environment set up so
my testing is limited to what I can do with MinGW on *nix.  The
scripts in `build_helpers/mingw_cross_build/` represents my current
degree of progress in this.

(If someone wants to take over Windows maintenance, let me know.)

That being said, here's what I know.

On Windows, we only build the SDL version because `ncurses` doesn't
support the Windows console and if we're going to use PDCurses anyway,
we may as well get the cool graphics.

My preferred toolchain is MinGW on [MSYS2](https://www.msys2.org/).
Since there are several variants of MSYS out there with different
levels of support, you may need to tweak your `config.mk` (see
below).  Nevertheless, here's what worked for me:

1. I installed MSYS2 with [Chocolatey](https://chocolatey.org/):
```sh
    choco install -y msys2
```

2. In an MSYS session, I used pacman to install all of the tools and libraries.
```sh
    pacman --noconfirm -S \
       mingw-w64-i686-gcc make git perl perl-modules zip unzip \
       mingw-w64-i686-ncurses mingw-w64-i686-SDL2 mingw-w64-i686-SDL2_ttf \
       gcc tar gzip
```
    I also had to add them to my `PATH`.

3. After that, it was just a simple matter checking out the source
   code and building it, as usual.
```sh
    cd relarn-pdcurses/sdl2
    make WIDE=Y
    cd ../../relarn/src
    make \
        PDCURSES=../../relarn-pdcurses/ \
        tarball
```

As with `macOS`, part of `make tarball` is fetching all of the
necessary DLLs so that it will work.  (You may need to manually include
the SDL DLLs as well; there may be a bug in the script that I haven't
yet tracked down.)



