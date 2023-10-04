
# Build configuration file.  In theory, this is the only file you need
# to edit.

# Comment out this line to enable assertions.  You only want to do
# this if you're developing or debugging.
#ASSERT_CFLAGS = -DDISABLE_ASSERT=1

# Set this to a PDCurses checkout with the sdl2 target built with
# WIDE=Y (or pass it to make as an argument).
#PDCURSES=../../relarn-pdcurses/

# This should have been set from the calling Makefile or on the
# command line, but if not you can set it here.  It's probably not a
# good idea though.
#
# SYS = linux-x86

# In theory (heh), you should not need to change anything beyond this
# line.


# Full platform ID (i.e. SYS but with -pdc appended if we're building
# against PDCurses+SDL2.)
ifeq ("$(PDCURSES)","")
	PLATFORM=$(SYS)
else
	PLATFORM=$(SYS)-pdc
endif


EXT=.bin		# Executable suffix; optional on Unix
SH_EXT=			# Shell script filename suffix
IS_WINDOWS=		# Empty unless target is stock (non-Cygwin) Windows
IS_MACGUI=		# Empty unless target is a macOS GUI app

ifeq ($(PLATFORM),linux-x86)					# E.g. Ubuntu + PC
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS=-Wno-format-truncation
	LIBS=-Wl,-Bstatic -lcurses -ltinfo -Wl,-Bdynamic -lm
else ifeq ($(PLATFORM),cygwin_nt-x86)		# Cygwin as target
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS=
	EXT=.exe
	SH_EXT=.sh
	LIBS=	-lcurses -lm
else ifeq ($(PLATFORM),linux-arm)			# Raspbian
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS=
	LIBS=-lcurses -lm
else ifeq ($(PLATFORM),darwin-x86)			# macOS
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS=-fno-spell-checking
	LIBS=-lcurses -lm
else ifeq ($(PLATFORM),darwin-x86-pdc)		# macOS + pdcurses
	CC=gcc
	PLATFORM_CFLAGS := -fno-spell-checking -I$(PDCURSES) -DUSE_PDCURSES=1 \
		`sdl2-config --cflags` -DPDC_WIDE
	LD=gcc
	LIBS=$(PDCURSES)/sdl2/pdcurses.a -lm `sdl2-config --libs` -lSDL2_ttf
	IS_MACGUI=yes
else ifeq ($(PLATFORM),linux-x86-pdc)		# E.g. Ubuntu + PC + pdcurses
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS=-Wno-format-truncation -I$(PDCURSES) -DUSE_PDCURSES=1 \
		`sdl2-config --cflags` -DPDC_WIDE=1
	LIBS= -lm $(PDCURSES)/sdl2/pdcurses.a `sdl2-config --libs` -lSDL2_ttf
else ifeq ($(PLATFORM),msys_nt-x86-pdc)		# Windows with MinGW via msys2
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS=-Wno-format-truncation -I$(PDCURSES) -DUSE_PDCURSES=1 \
		`sdl2-config --cflags` -DPDC_WIDE=1 -DWIN32_LEAN_AND_MEAN=1 -mwindows
	PLATFORM_LDFLAGS=-mwindows
	EXT=.exe
	SH_EXT=.sh
	LIBS= $(PDCURSES)/sdl2/pdcurses.a `sdl2-config --static-libs` -lSDL2_ttf \
		-lsecur32
	IS_WINDOWS=yes
else ifeq ($(PLATFORM),cross-mswin)			# Windows cross compiler (doesn't work)
	CC=x86_64-w64-mingw32-gcc
	LD=$(CC)
	PLATFORM_CFLAGS=-Wno-format-truncation -I$(PDCURSES) -DUSE_PDCURSES=1 \
		`sdl2-config --cflags` -DPDC_WIDE=1 -DWIN32_LEAN_AND_MEAN=1
	LIBS= -lm $(PDCURSES)/sdl2/pdcurses.a `sdl2-config --libs` -lSDL2_ttf
	IS_WINDOWS=yes
else ifeq ($(PLATFORM),msys_nt-x86)			# Windows console (unsupported)
# We currently don't support TTY-mode on Windows
$(error PDCURSES must be set when building for Windows $(PLATFORM))
else
$(error No settings for $(SYS) ($(PLATFORM)))
endif
