# Building ReLarn

ReLarn should be easy to build on any sufficiently Unix-like system.
If it doesn't work out of the box, the build scripts are just small,
hand-written Makefiles and shell scripts so they're easy to fix.

It has been successfully built on Ubuntu Linux, Raspbian on a
Raspberry Pi 3, macOS with MacPorts, and Cygwin on Windows 7.

The native Windows version requires MSYS and MINGW; I use
[MSYS2](https://www.msys2.org/) and gcc.

The macOS app uses only command-line tools, scripts and trickery.


## Overview

To build basic ReLarn, you will need:

* gcc, clang or some other C compiler that supports C99 and the common
  Unix compiler arguments.
* ncurses version 6 or better.  (Earlier versions may also work.)
* GNU Make
* A sufficiently Unix-ish environment including `bash` and `perl`
  (optional; `pod2html` is used to generate the man page).  On
  Windows, MSYS is sufficient

To build the windowed version of ReLarn, you will also need

* PDCurses 3.8 (cloned [here](https://gitlab.com/relarn/relarn-pdcurses.git) 
  for your convenience.)
* SDL 2.0.5 and libsdl2_ttf and all their dependencies

And you can skip ncurses in this case.

The Makefile accepts three arguments:

* `INST_ROOT` is the place to install the game.  Defaults 
   to `~/apps/relarn-<version>/`.
* `RELEASE` should be `yes`; disables certain overcautious
  compile-time errors.
* `PDCURSES` is the path to your PDCurses build if you wish to build
  the windowed version; otherwise, it should be left out.

These are typically set on the `make` command line.

You may also want to look at
[this project](https://gitlab.com/relarn/relarn-ci-build).  It
contains scripts that will build ReLarn from sources in Gitlab's CI
environment and so may be a useful reference.

    
## Building the TTY version on Linux

To build on Linux, first install the dependencies as outlined above.
On my reference system (Ubuntu 18LTS), it's a simple matter of typing:

    sudo apt-get install libncurses-dev gcc make perl

At this point, you just run make in `src` with the appropriate
parameters:

    cd relarn/src
    make RELEASE=yes INST_ROOT=~/my_apps/relarn-2.2.0
    make RELEASE=yes INST_ROOT=~/my_apps/relarn-2.2.0 install

This will install relarn in directory `~/my_apps/relarn-2.2.0`.

If you want to do the standard Unix thing and put it in a system
location, you can do that too:

    cd relarn/src
    make RELEASE=yes INST_ROOT=/usr/local/games
    sudo make RELEASE=yes INST_ROOT=/usr/local/games install

Note that different major versions of ReLarn aren't savefile
compatible, so upgrading in this way will make it impossible to finish
your current game.  You can work around this by installing them in
separate versioned subdirectories and making symlinks to the relevant
`bin` directory if that's an issue.


## Building the SDL version on Linux

Building the SDL version on Linux requires most of the same
dependencies as the TTY version (minus ncurses) plus SDL and SDL_ttf:

    apt install libsdl2-dev libsdl2-ttf-dev

(I also had to explicitly install `libglu1-mesa-dev` but I think
that's due to a packaging bug.)

Once those are installed, you will need to build PDCurses.  I use the
copy I made (linked above):

    git clone https://gitlab.com/relarn/relarn-pdcurses.git

Once it's checkout out, build the sdl2 interface with TrueType fonts
enabled:

    cd relarn-pdcurses/sdl2
    make WIDE=Y
    cd ../..

Now, you can build ReLarn:

    cd relarn/src
    make clean      # Only needed if there was a previous build.
    make PDCURSES=../../relarn-pdcurses/ \
         RELEASE=yes \
         INST_ROOT=~/my_apps/relarn-2.2.0 \
         install

Building a binary release is done in mostly the same way but you
should build target `install_dist`.  You will also need to tar and/or zip
the installation directory yourself.  Example:

    cd relarn/src
    make clean      # Only needed if there was a previous build.
    mkdir relarn-2.2.0
    make PDCURSES=../../relarn-pdcurses/ \
         RELEASE=yes \
         INST_ROOT=relarn-2.2.0 \
         install_dist
    tar cvf relarn-2.2.0.tar relarn-2.2.0/
    gzip -9 relarn-2.2.0.tar

Using `install_dist` instead of `install` is not strictly necessary on
Linux but makes a difference on Windows and macOS.


## Building for macOS

Building on macOS is mostly the same as Linux. You will need to
install Xcode and the command-line utilities.  The other dependencies
are all available on MacPorts and (I assume) Brew.  After that, just
follow the steps outlined above.

Note that it is important to use `make install_dist` when creating a
redistributable build because that will find and include the necessary
shared libraries.

(If you don't want to install Xcode, you can probably get what you
need from MacPorts or Brew but I haven't tried it that way.)

You can also build ReLarn as an App bundle and package it in a dmg
file.

    make PDCURSES=../../relarn-pdcurses/ \
         RELEASE=yes \
         app

or

    make PDCURSES=../../relarn-pdcurses/ \
         RELEASE=yes \
         dmg

(`make dmg` implies `make app`.)

Note that this is **EXPERIMENTAL** and may well not play nice with
Apple's app validation stuff.

(Actually, the Mac stuff is generally harder to validate because
there's no easy and cheap way to create a clean environment.  For
Linux, there's Docker and Windows runs easily enough in VirtualBox but
getting a fresh macOS install up and running just to do a clean build
or a test run is way more work and expense than I'm willing to do
unpaid.  So even though my principle development machine right now is
a MacBook, this platform is going to get the worst support, just
because "works on my machine" is the best validation I can do.)



## Building for Microsoft Windows

On Windows, I build with [MSYS2](https://www.msys2.org/) using gcc.
Since there are several variants of MSYS out there with different
levels of support, you may need to tweak your `config.mk` (see
below).  Nevertheless, here's what works for me:

Firstly, I installed MSYS2.  If you have
[Chocolatey](https://chocolatey.org/) installed, it's as easy as:

    choco install -y msys2

but you can also just install it the old-fashioned way.

Once MSYS2 is installed, open the console and install the
dependencies:

    pacman --noconfirm -S \
       mingw-w64-i686-gcc make git perl perl-modules zip unzip \
       mingw-w64-i686-ncurses mingw-w64-i686-SDL2 mingw-w64-i686-SDL2_ttf \
       gcc tar gzip

After this, you need to add the 32-bit tools to the path (along with
core_perl for some reason):

    export PATH="/mingw32/bin:/usr/bin/core_perl/:$PATH"

Once that's done, you can checkout `relarn` and `relarn-pdcurses` and
build them in the usual way:

    cd relarn-pdcurses/sdl2
    make WIDE=Y
    cd ../../relarn/src
    make \
        PDCURSES=../../relarn-pdcurses/ \
        INST_ROOT=my_install_dir \
        install

As above, you should make target `install_dist` if you are compiling
for binary redestribution.

Note that the native Windows version **does not support TTY mode**.
It will **always** open its own window.


### Other Platforms (and/or Fixing Stuff)

I have also managed to occasionally get ReLarn built on Raspbian and
Cygwin.  In both cases, the usual *nix build procedure works
reasonably well.  If not, it usually comes down to just editing
`src/config.mk` and/or `build_helpers/platform-id.sh`.

`platform-id.sh` is a simple `bash` script that prints out a string
identifying your OS and CPU architecture (think GNU `config.guess` but
not as good).  This gets called by `make` in the file `src/config.mk`
which the `Makefile` includes.

`config.mk` is mostly just a giant `ifeq ... else ifeq` block that
sets the necessary variables (command-line options, etc) for each
flag.

If you want to add support for a new platform, simply add it to
`platform-id.sh` and `config.mk` and there you go.

(If you just want to PLAY THE STUPID GAME, you could just delete most
config.mk minus the closest set of variables that work and then tweak
those 'til the game compiles.  But you knew that already, right?)
