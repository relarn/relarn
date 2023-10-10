#!/bin/bash

# Run relarn after first setting RELARN_INSTALL_ROOT so that it can
# find its resource files.

set -e

# Sanity check
if [ -z "$BASH_VERSION" ]; then
    echo "This script must be run with bash."
    exit 1
fi

# Hack! Windows wants .exe for the extension so the Makefile does
# that; we just detect Windows here and do the same thing.
bin=.bin
if (uname -s | grep -q '^CYGWIN_NT'); then
    bin=.exe
fi

# Find the actual installation directory if this script was run via a
# symlink.  (macOS's 'readlink' doesn't support `-f` so we need to find the
# actual path the hard way.)
this_script="${BASH_SOURCE[0]}"
while [ -L "$this_script" ]; do
    this_script=$(readlink "$this_script")
done

RELARN_INSTALL_ROOT=$(dirname "$this_script")/..
RELARN_INSTALL_ROOT=$(cd -P "$RELARN_INSTALL_ROOT"; pwd)

export RELARN_INSTALL_ROOT

# For macOS, we need to set the dylib load path to find the shared libs.
if [[ "`uname -s`" = "Darwin" ]]; then
    export DYLD_LIBRARY_PATH="$RELARN_INSTALL_ROOT/lib/relarn/"
fi

if [ ! -x "$RELARN_INSTALL_ROOT/lib/relarn/relarn$bin" ]; then
    echo "Incomplete relarn installation."
    exit 1
fi

exec "$RELARN_INSTALL_ROOT/lib/relarn/relarn$bin" "$@"


