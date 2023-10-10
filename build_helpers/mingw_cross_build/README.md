# MinGW cross-build script

(This is **EXPERIMENTAL**; use at own risk!)

This script will (attempt to) build ReLarn for Windows from a *nix
host using the MinGW cross-compilers.  It will likely not work for you
without a bunch of fiddling but I'm providing it as a starting point
for those so inclined.

Briefly, it:

1. Fetches the SDL2 and SDL2_ttf development releases for MinGW.

2. Fetches PDCurses from the forked repository and builds it with
   MinGW.

3. Puts everything in `relarn/_third_party/`.

4. Invokes `make` in `relarn/src` with `PDCURSES` and `MINGW_PREFIX`
   set correctly and passes its remaining arguments to it.

Steps 1-3 are skipped if they've already been done so you can treat it
as a wrapper around `make` that builds for Windows.





