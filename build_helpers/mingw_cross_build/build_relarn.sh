#!/bin/bash

# Script to build relarn on *nix using the MinGW cross compiler.


#ARCH=i686
ARCH=x86_64

MINGW_PREFIX=$ARCH-w64-mingw32

# We have a slight modified version of PDCurses that we use instead of
# the main one.
PDCURSES_REPO=https://gitlab.com/relarn/relarn-pdcurses.git
RELARN_PDCURSES_RELEASE=3.8-relarn-config

export DEBUG=Y      # Build PDCurses with debugging enabled


###########################################################################

set -e

# Get absolute paths to this script and the project root, then move to
# the project root
cd -P "$(dirname "${BASH_SOURCE[0]}")"
SCRIPT_ROOT=`pwd`
cd ../..
ROOT=`pwd`

CACHE_DIR="$ROOT/_third_party/cache"
SDL_DIR="$ROOT/_third_party/libsdl"

# Load helper code
. "$SCRIPT_ROOT/lib/fetch_mingw_sdl.sh"
. "$SCRIPT_ROOT/lib/build_pdcurses.sh"

[ -d _third_party ] || mkdir _third_party
cd _third_party

if [ ! -d $SDL_DIR ]; then
    fetch_mingw_sdl $ARCH
else
    echo "Found libsdl; not refetching."
fi

export PATH="$SDL_DIR/bin:$PATH"
if sdl2-config --libs 2>&1 > /dev/null ; then
    true
else
    echo "sdl2-config not in PATH!"
    exit 1
fi


# Fetch and build pdcurses
checkout_pdcurses
build_pdcurses $MINGW_PREFIX $SDL_DIR

for d in $SDL_DIR/bin/*.dll; do
    bd=$(basename $d)
    [ -f $src/$bd ] && echo "Copying over $bd" && cp $d src/$bd
done

# And do the compile
cd $ROOT/src
make PDCURSES=$ROOT/_third_party/relarn-pdcurses \
     MINGW_PREFIX=$MINGW_PREFIX \
     SDL_PREFIX="--prefix='$SDL_DIR'" \
     "$@"
