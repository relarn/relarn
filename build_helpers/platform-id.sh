#!/bin/bash

# Attempt to guess the current platform and print out a normalized
# name for use by Makefile et. al.  Only handles platforms test for in
# config.mk.  (This is sort of a low-rent clone of config.guess).

set -e

os=$(uname -s | tr [A-Z] [a-z])

# Clean up the various cygwin flavours
case $os in
    msys_nt*)   os=msys_nt ;;
    mingw*)     os=msys_nt ;;
    cygwin_nt*) os=cygwin_nt ;;
esac


arch=$(uname -m)

case $arch in
    # It turns out we can lump all the x86 systems together
    i[0-9]86)   arch=x86 ;;
    x86_64)     arch=x86 ;;

    # All arm variants get called "arm"
    arm*)       arch=arm ;;
esac

echo "$os-$arch"
