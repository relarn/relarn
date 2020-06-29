#!/bin/bash

# Windows+MSYS: Copy all non-system shared libraries that relarn.bin
# depends on.  Used for creating redistributable binary packages.

set -e

function get_deps_for() {
    local bin="$1"
    
    echo "Scanning $bin..."

    ldd $bin | \
        perl -ne '/=>\s*(\S+)\s+\(/ && print "$1\n"' | \
        while read lib; do
            [ "${lib:0:6}" = "/mingw" ] || continue

            if [ -f "$(basename $lib)" ]; then
                echo "Already have $lib"
            else
                echo "Copying $lib"
                cp $lib .
            fi
        done
}


get_deps_for ./relarn.exe

nlibs=""
while true; do
    compgen -G '*.dll' || break
    for lib in *.dll; do
        get_deps_for $lib
    done

    nnlibs=$(ls -1 *.dll | wc -l)
    if [ "$nnlibs" = "$nlibs" ]; then
        break
    fi
    nlibs=$nnlibs
done
