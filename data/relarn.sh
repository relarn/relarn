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

export RELARN_INSTALL_ROOT=$(cd -P "$(dirname "${BASH_SOURCE[0]}")/.." ; pwd)

if [ ! -x "$RELARN_INSTALL_ROOT/lib/relarn/relarn$bin" ]; then
    echo "Incomplete relarn installation."
    exit 1
fi

exec "$RELARN_INSTALL_ROOT/lib/relarn/relarn$bin" "$@"


