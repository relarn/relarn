#!/bin/bash

# macOS: Copy all non-system shared libraries that relarn.bin depends on.
# 
# Warning: this works by excluding certain paths so it's possible that
# I haven't been thorough enough and you're getting system libs.  So
# check on that.

set -e


function get_deps_for() {
    local bin="$1"
    
    echo "Scanning $bin..."
    
    otool -L "$bin" | tail -n +2 | sed -e 's/^[         ]*//' -e 's/ .*//' | \
        while read lib; do
            (echo $lib |grep -q '^\(/System\|/usr/lib\)' ) && continue
            if [ -f "$(basename $lib)" ]; then
                echo "Already have $lib"
            else
                echo "Copying $lib"
                cp $lib .
            fi
        done
}


get_deps_for ./relarn.bin

nlibs=""
while true; do
    compgen -G '*.dylib' || break
    for lib in *.dylib; do
        get_deps_for $lib
    done

    nnlibs=$(ls -1 *.dylib | wc -l)
    if [ "$nnlibs" = "$nlibs" ]; then
        break
    fi
    nlibs=$nnlibs
done
