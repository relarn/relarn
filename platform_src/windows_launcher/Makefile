
SYS := $(shell ../../build_helpers/platform-id.sh)

include ../../src/config.mk

.suffixes: .exe

all: windows_launcher.exe

windows_launcher.exe : windows_launcher.c windows_launcher.rc
ifeq ($(IS_WINDOWS),yes)
	windres windows_launcher.rc -O coff -o windows_launcher.res
	$(CC) -DWIN32_LEAN_AND_MEAN=1 -mwindows -lm windows_launcher.c \
		windows_launcher.res -o windows_launcher.exe
endif


clean:
	-rm *.o *.exe *.res
