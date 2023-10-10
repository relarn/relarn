

function checkout_pdcurses() {
    cd $ROOT/_third_party

    if [ ! -d relarn-pdcurses ]; then
        echo "Cloning..."
        git clone $PDCURSES_REPO relarn-pdcurses
    fi

    cd relarn-pdcurses
    git checkout $RELARN_PDCURSES_RELEASE
    cd $ROOT
}

function build_pdcurses() {
    local mingw_prefix=$1
    local sdl_root=$2
    local clean=$3

    #local mingw_prefix=$arch-w64-mingw32

    cd "$ROOT/_third_party/relarn-pdcurses/sdl2"

    ( set -e

      export CC=$mingw_prefix-gcc

      # SDL2.0.x's sdl2-config has a bug
      sdl2-config --prefix=$sdl_root --cflags
      sflags=$(sdl2-config --prefix=$sdl_root --cflags)
	  slibs=$(sdl2-config --prefix=$sdl_root --libs)

      echo sflags=$sflags
      echo slibs=$slibs
      #export STRIP=$mingw_prefix-strip

      [ -n "$clean" ] && make clean

      make SFLAGS="$sflags" SLIBS="$slibs" WIDE=Y all

      # The makefile doesn't (currently) run ranlib so we need to do
      # it by hand.
      $mingw_prefix-ranlib pdcurses.a

      # These are sometimes helpful as a smoketest but aren't
      # necessary.  The Makefile may fail if our local 'strip' isn't
      # compatible.  TODO: strip
      #make -j $THREADS WIDE=Y demos

      cp $SDL_DIR/bin/*.dll .
    )
}
