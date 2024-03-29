#!/bin/bash

# Script to install relarn.bin and related files into a directory,
# creating the standard file structure.  This is run by the Makefile,
# from which it was originally extracted.

set -e

INST_ROOT="$1"
bin="$2"            # Filename extension for the binary
is_dist="$3"        # Non-empty if creating a binary distribution
is_windows="$4"     # Non-empty if targetting plain (non-Cygwin) Windows
is_crossbuild="$5"  # Non-empty if cross-compiling for Windows

LIBDIR="$INST_ROOT/share/relarn/lib"
DOCDIR="$INST_ROOT/share/relarn/doc"
EXEDIR="$INST_ROOT/lib/relarn"
BINDIR="$INST_ROOT/bin"
SCOREDIR="$INST_ROOT/var/relarn"
MANDIR="$INST_ROOT/share/man/man6"

# Sanity check for testing
if [ -z "$INST_ROOT" ]; then
    echo "No installation directory given."
    exit 1
fi

# Create the data directory and copy over the game data
mkdir -p "$LIBDIR"
for d in Umaps Ufortune Uhelp Uintro Ujunkmail relarnrc.sample \
         icon/relarn-icon.bmp
do
	cp ../data/$d "$LIBDIR"
	chmod 664 "$LIBDIR/`basename $d`"
done
cp -r ../data/fonts "$LIBDIR/"
chmod 664 "$LIBDIR"/fonts/*


# Create the score directory and empty scorefile.
mkdir -p "$SCOREDIR"
touch "$SCOREDIR/Relarn-scoreboard"     # TODO: derive this from sources
chmod a+rw "$SCOREDIR/Relarn-scoreboard"

# Create the executable directories and install the executable and
# launch script.
mkdir -p "$EXEDIR" "$BINDIR"

[ -f "$EXEDIR/relarn$bin" ] && rm -f "$EXEDIR/relarn$bin"  # might be read-only
cp relarn$bin "$EXEDIR/relarn$bin"
chmod a-w "$EXEDIR/relarn$bin"        # Disables the debug menu


# Fetch the shared libs on Windows and macOS
if [ -n "$is_dist" ] \
       && [ -z "$is_crossbuild" ] \
       && [ -n "$is_windows" -o  "$(uname)" = Darwin ]
then
    script=fetch_osx_dylibs.sh
    [ -n "$is_windows" ] && script=fetch_msys_dlls.sh

    ( set -e
      scriptdir=$(cd `dirname $BASH_SOURCE[0]`; pwd)
      cd "$EXEDIR"
      bash $scriptdir/$script
    )
fi

# If it's a cross-build, we only need the SDL DLLs.  I think.
if [ -n "$is_crossbuild" ]; then
    # This only works because sdl2-config needs to be in the path.
    echo "Copying SDL DLLs."
    sdl_path=$(dirname $(which SDL2.dll))
    cp "$sdl_path"/*.dll "$EXEDIR/"
fi


# Install the launcher
if [ -n "$is_windows" ]; then
    for b in relarn relarn-scores relarn-winning-scores; do
        cp ../platform_src/windows_launcher/windows_launcher.exe "$BINDIR/$b.exe"
    done
else
    cp ../data/relarn.sh "$BINDIR/relarn"
    chmod a+x "$BINDIR/relarn"
fi

# Create the manpage. We skip this step if pod2man isn't installed so
# that you don't need to install Perl just to generate a manpage.
mkdir -p "$DOCDIR" "$MANDIR"
if command -v pod2man > /dev/null; then
    pod2man -s 6 -r ReLarn ../doc/relarn.pod > ../doc/relarn.6
    cp ../doc/relarn.6 "$MANDIR"
fi

# Create the doc directory and copy over the docs.
for f in     ../doc/historical/Ularn-spoilers.txt \
             ../LICENSE.txt \
             ../AUTHORS.txt \
             ../Copyright.txt \
             fov/COPYING-libfov
do
         cp $f "$DOCDIR"
done
