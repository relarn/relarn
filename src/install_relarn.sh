#!/bin/bash

# Script to install relarn.bin and related files into a directory,
# creating the standard file structure.  This is run by the Makefile,
# from which it was originally extracted.

set -e

INST_ROOT="$1"
bin="$2"            # Filename extension for the binary
sh_ext="$3"         # Filename extension for the launcher script

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
for d in Umaps Ufortune Uhelp Uintro Ujunkmail; do
	cp ../data/$d "$LIBDIR"
	chmod 664 "$LIBDIR/$d"
done

# Create the score directory and empty scorefile.
mkdir -p "$SCOREDIR"
touch "$SCOREDIR/Relarn-scoreboard"     # TODO: derive this from sources
chmod a+rw "$SCOREDIR/Relarn-scoreboard"

# Create the executable directories and install the executable and
# launch script.
mkdir -p "$EXEDIR" "$BINDIR"

[ -f "$EXEDIR/relarn$bin" ] && rm -f "$EXEDIR/relarn$bin"  # might be read-only
cp relarn$bin "$EXEDIR/relarn$bin"
strip "$EXEDIR/relarn$bin"
chmod a-w "$EXEDIR/relarn$bin"        # Disables the debug menu

cp ../data/relarn.sh "$BINDIR/relarn$sh_ext"
chmod a+x "$BINDIR/relarn$sh_ext"


# Create the manpage. We skip this step if pod2man isn't installed so
# that you don't need to install Perl just to generate a manpage.
mkdir -p "$DOCDIR" "$MANDIR"
if command -v pod2man > /dev/null; then
    pod2man -s 6 -r ReLarn ../doc/relarn.pod > relarn.6
    cp relarn.6 "$MANDIR"    
fi

# Create the doc directory and copy over the docs.
for f in ../doc/relarnrc.sample \
             ../doc/historical/Ularn-spoilers.txt \
             ../LICENSE.txt \
             ../AUTHORS.txt \
             ../Copyright.txt
do
         cp $f "$DOCDIR"
done       
