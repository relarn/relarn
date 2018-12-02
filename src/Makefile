
#   Import build configuration
include config.mk

#	Import release version numbers
include version.mk

#   Build details, part of the extended version info
DATE := $(shell date +'(%Y-%m-%d)')
COMMIT_ID := $(shell git rev-parse --short HEAD)
PLATFORM := $(shell uname -psr)

#  	name of program
PROGRAM	= relarn$(EXT)

# 	for console UI
SYS_LIBRARIES=	-lcurses 

#   link flags
LDFLAGS= -g -Wall

#   compile flags
#   (we use gnu99 instead of c99 in order to get POSIX definitions.)
CFLAGS= -std=gnu99 -g -Wall -Wno-comment -Werror $(PLATFORM_CFLAGS) \
	$(ASSERT_CFLAGS)

#   defines:
DEFINES=-DINST_ROOT="\"$(INST_ROOT)\"" -DPLATFORM_ID="\"$(PLATFORM)\""	\
	-DBUILD_DATE="\"$(DATE)\"" -DCOMMIT_ID="\"$(COMMIT_ID)\""			\
	-DRELARN_VERSION="\"$(RELARN_VERSION)\""							\
	-DRELARN_PATCHLEVEL="\"$(RELARN_PATCHLEVEL)\""

#   the source files
SRCS1 = action.c bank.c bill.c cast.c create.c debug.c diag.c	\
display.c fortune.c game.c player.c help.c look.c		\
main.c monster.c movem.c object.c os.c map.c score_file.c show.c	\
sphere.c store.c settings.c ui.c textbuffer.c lrs.c \
picklist.c util.c school.c stringbuilder.c text_template.c

#   the object files
OBJS1	= $(SRCS1:.c=.o)

#   data files
LIBFILES=Uhelp Umaps Ufortune Uintro


all: $(PROGRAM)

over: clean all

$(PROGRAM): $(OBJS1)
	$(CC) -o $@.tmp $(LDFLAGS) $(OBJS1)  $(SYS_LIBRARIES)
	-rm -f $@
	mv $@.tmp $@

.c.o:
	$(CC) -c $(CFLAGS) $(DEFINES) $<


install: $(PROGRAM)
	./install_relarn.sh "$(INST_ROOT)" $(EXT) $(SH_EXT)

distbin: $(PROGRAM)
	(tmp=`mktemp -d`; \
	 ver=$(RELARN_VERSION).$(RELARN_PATCHLEVEL) \
	 dir=relarn-$$ver; \
     ./install_relarn.sh $$tmp/$$dir $(EXT) $(SH_EXT); \
	 (cd $$tmp; tar cvf relarn-$$ver-bin.tar $$dir; gzip -9 *.tar; \
      rm -rf $$dir) ; \
	 mv $$tmp/*.tar.gz .. ; \
	 rmdir $$tmp )

lint:
	lint $(SRCS1) $(LINTLIBS)

clean:
	rm -f $(PROGRAM) $(OBJS1) core.[0-9]+ deps.mk ../doc/relarn.6

distclean: clean
	rm -f *~ .#*

tags: $(SRCS1) *.h
	etags $(SRCS1) *.h

# Actual dependencies: these are generated with 'gcc -MM' and
# included.  If your compiler doesn't support this 
deps.mk:
	gcc -MM $(DEFINES) $(SRCS1) > deps.mk

include deps.mk
