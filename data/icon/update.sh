#!/bin/bash

# Convert the master icon (relarn-icon.png) to the various formats we
# need.  Run this if you've changed the icon.
#
# This is not part of the build process so that most people won't need
# to install ImageMagick just to compile ReLarn.

set -e

# Windows-style icon for the Windows launchers
convert relarn-icon.png relarn-icon.ico

# BMP for PDCurses+SDL to load as a runtime application icon
convert relarn-icon.png relarn-icon.bmp

# OSX Icon (Requires makeicns from MacPorts.)
makeicns -in relarn-icon.png -out relarn-icon.icns
