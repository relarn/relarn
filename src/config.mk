# Build configuration file.  In theory, this is the only file you need
# to edit.

# Absolute path to the installation directory. This also gets
# hardcoded in the executable but may be overridden by the launch
# script.
INST_ROOT = $(HOME)/apps/relarn
#INST_ROOT = /usr/local/games/

# Comment out this line to enable assertions.  You only want to do
# this if you're developing or debugging.
ASSERT_CFLAGS = -DDISABLE_ASSERT=1



# In theory (heh), you should not need to change anything beyond this
# line.

SYS=$(shell uname -s | sed -e 's/-[.0-9]*$$//')-$(shell uname -m)

EXT=.bin
SH_EXT=

ifeq ($(SYS),Linux-x86_64)			# E.g. Ubuntu + PC
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS= 
else ifeq ($(SYS),CYGWIN_NT-i686)	# 32-bit Cygwin
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS=
	EXT=.exe
	SH_EXT=.sh
else ifeq ($(SYS),CYGWIN_NT-x86_64)	# 64-bit Cygwin
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS=
	EXT=.exe
	SH_EXT=.sh
else ifeq ($(SYS),Linux-armv7l)		# E.g. Raspbian + Raspberry Pi 
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS
else ifeq ($(SYS),Darwin-x86_64)	# E.g. macOS + Macintosh
	CC=gcc
	LD=gcc
	PLATFORM_CFLAGS=-fno-spell-checking
else
$(error No settings for $(SYS))
endif
