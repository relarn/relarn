
#	Import release version numbers
include version.mk

#	Determine the default platform if not overridden
SYS := $(shell ../build_helpers/platform-id.sh)

#   Import build configuration
include config.mk

# Option '-Werror' is really helpful when developing but it causes
# problems for third parties trying to build from source.  We leave it
# the default but let the user disable it and update the build
# instructions to reflect that.  (We also use this as a place to put
# optimization flags, but be aware that higher optimization levels can
# reveal more warnings.)
RELEASE=no
ifeq ($(RELEASE),no)
WERROR=-Werror
OPT_CFLAG=-O0
else
WERROR=-Wno-error
OPT_CFLAG=-O
endif

#   Build details, part of the extended version info
DATE := $(shell date +'(%Y-%m-%d)')
COMMIT_ID := $(shell \
	(command -v git 2>&1 >/dev/null && git rev-parse --short HEAD) || \
	echo "")

#	optimization.  Keep at -O2 unless debugging

#  	name of program
PROGRAM	= relarn$(EXT)

#   link flags
LDFLAGS= -g -Wall $(PLATFORM_LDFLAGS)

#   compile flags
#   (we use gnu99 instead of c99 in order to get POSIX definitions.)
CFLAGS= -std=gnu99 -g -Wall -Wno-comment $(WERROR) $(PLATFORM_CFLAGS)   \
	$(ASSERT_CFLAGS) $(OPT_CFLAG)

#   defines:
DEFINES=-DINST_ROOT="\"$(INST_ROOT)\"" -DPLATFORM_ID="\"$(SYS)\""	\
	-DBUILD_DATE="\"$(DATE)\"" -DCOMMIT_ID="\"$(COMMIT_ID)\""			\
	-DRELARN_VERSION="\"$(RELARN_VERSION)\""							\
	-DRELARN_PATCHLEVEL="\"$(RELARN_PATCHLEVEL)\""


#	common source files
COMMON_SRC = action.c bank.c bill.c cast.c create.c debug.c diag.c	\
display.c fortune.c game.c player.c help.c look.c		\
main.c monster.c movem.c object.c os.c map.c score_file.c show.c	\
sphere.c store.c settings.c ui.c textbuffer.c lrs.c \
picklist.c util.c school.c stringbuilder.c text_template.c fov/fov.c \
internal_assert.c savegame.c

#	Sources that aren't used in *this* configuration
ALT_SRC = 


#   Curses-extension-specific sources.  We default to ncurses unless
#   PDCurses has been specified.
ifeq ("$(PDCURSES)","")
	CURSES_EXT = curses_extensions_ncurses.c
	ALT_SRC += curses_extensions_pdcurses.c
else
	CURSES_EXT = curses_extensions_pdcurses.c
	ALT_SRC += curses_extensions_ncurses.c
endif

#	OS-specific source(s).
ifeq ($(IS_WINDOWS),yes)
	OS_EXT = os_windows.c
	ALT_SRC = os_unix.c
else
	OS_EXT = os_unix.c
	ALT_SRC += os_windows.c
endif


#   the active source files
SRCS1 = $(COMMON_SRC) $(CURSES_EXT) $(OS_EXT)

#   the object files
OBJS1	= $(SRCS1:.c=.o)

#	all source files
ALL_SRCS = $(SRCS1) $(ALT_SRC)

#   data files
LIBFILES=Uhelp Umaps Ufortune Uintro


all: $(PROGRAM)
	@echo "commit-id (if present) is '$(COMMIT_ID)'"

over: clean all

$(PROGRAM): $(OBJS1)
	$(CC) -o $@.tmp $(LDFLAGS) $(OBJS1)  $(LIBS)
	-rm -f $@
	mv $@.tmp $@

.c.o:
	$(CC) -c $(CFLAGS) $(DEFINES) $< -o $@


install: $(PROGRAM)
	(cd ../platform_src/windows_launcher; make SYS=$(SYS) all)
	../build_helpers/install_relarn.sh "$(INST_ROOT)" $(EXT) $(IS_DIST) \
		$(SH_EXT) $(IS_WINDOWS)

install_dist:
	$(MAKE) IS_DIST=yes install

ifeq ($(IS_MACGUI),yes)
app:
	( [ -d ReLarn.app ] && rm -rf ReLarn.app ; \
	  cp -a ../platform_src/osx_app_skel/ReLarn.app . )
	make INST_ROOT=ReLarn.app/Contents/MacOS/ install_dist
else
app:
	@echo "This target requires a UI build on MacOS."
	false
endif

clean:
	-rm -f $(PROGRAM) $(OBJS1) core.[0-9]+ deps.mk ../doc/relarn.6
	-(cd ../platform_src/windows_launcher; make clean)
	-( [ -d ReLarn.app ] && rm -rf ReLarn.app )


distclean: clean
	rm -f *~ .#*

tags: $(ALL_SRCS) *.h
	etags $(ALL_SRCS) *.h

# Actual dependencies: these are generated with 'gcc -MM' and
# included.  This assumes $CC is sufficiently gcc-like.
deps.mk:
	$(CC) -MM $(CFLAGS) $(DEFINES) $(SRCS1) > deps.mk

include deps.mk
