#!/bin/bash

set -e

function get() {
    local url="$1"
    local sum="$2"
    local arch="$3"

    [ -d "$CACHE_DIR" ] || mkdir "$CACHE_DIR"

    echo "Retrieving $url"
    local filename="$CACHE_DIR"/$(basename "$url")
    if [ -f $filename ]; then
        echo "Already have it; using $filename"
    else
        wget "$url" -O "$filename.$$"
        mv "$filename.$$" "$filename"
    fi

    echo "Checking hashes."
    local localsum=$(shasum "$filename" | perl -npe 's/\s.*//')
    if [ "$localsum" != "$sum" ]; then
        echo "Hash mismatch for $url! ([$localsum] != [$sum])"
        exit 1
    else
        echo "OK!"
    fi

    [ -d libsdl ] || mkdir libsdl

    echo "Unpacking..."
    tar xf $filename
    dir=$(echo SDL2*)
    if [ ! -d "$dir" ]; then
        echo "Can't find SDL dir."
        exit 1
    fi

    # Copying to libsdl
    cp -a $dir/$arch-w64-mingw32/* libsdl/

    rm -rf $dir
}

function fetch_mingw_sdl() {
    local arch="$1"

    local tmpdir=_tmp_$$
    local github=https://github.com/libsdl-org/

    mkdir $tmpdir
    cd $tmpdir
    echo "Creating $tmpdir"

    # Fetch SDL
    get \
        $github/SDL/releases/download/release-2.0.10/SDL2-devel-2.0.10-mingw.tar.gz \
        b6d2673bcc0e9e7d3903830908244ff185dac3f6 \
        $arch

    # Fetch SDL_ttf
    get \
        $github/SDL_ttf/releases/download/release-2.0.15/SDL2_ttf-devel-2.0.15-mingw.tar.gz \
        d658b617cdd72be665c6b00c69f42a8631f0d5ba \
        $arch

    mv libsdl "$SDL_DIR"
    cd ..
    rm -rf "$tmpdir"
    echo "Removed $tmpdir"
}
