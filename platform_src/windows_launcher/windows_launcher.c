// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <windows.h>
#include <dirent.h>
#include <process.h>


// Get the path to this executable and split it into the executable
// name and path to the installation root.  We assume that 'name' and
// 'root' point to buffers that are at least MAX_PATH characters long.
static void
getNameAndRoot(char *name, char *root) {

    char exepath[MAX_PATH];
    DWORD sz = GetModuleFileNameA(NULL, exepath, sizeof(exepath));
    if (sz == 0 || sz == sizeof(exepath)) {
        goto failure;
    }// if

    // Find the last slash in the path
    char *namePtr = &exepath[strlen(exepath) - 1];
    while(namePtr > exepath && *namePtr != '\\') {
        --namePtr;
    }
    if (namePtr == exepath) { goto failure; }

    // Split on the slash
    *namePtr = 0;
    ++namePtr;

    // Recover the name
    strncpy(name, namePtr, MAX_PATH);
    
    // We know the path will end with 'bin/' so we drop that.  We
    // actually walk back to the previous slash because that makes
    // this easier to test.
    char *pathEnd = &exepath[strlen(exepath) - 1];
    while(*pathEnd != '\\') {
        --pathEnd;
        if (pathEnd <= exepath) { goto failure; }
    }
    *pathEnd = 0;
    
    // And then that's the path.
    strncpy(root, exepath, MAX_PATH);

    // Done!
    return;

failure:
    MessageBoxA(NULL,
                "Can't find ReLarn installation. "
                "Maybe move it somewhere near c:\\?",
                
                "Error!", MB_OK | MB_ICONERROR);
    exit(1);
}// getNameAndRoot


static void
downcase(char *str) {
    for (int n = 0; str[n]; n++) {
        str[n] = tolower(str[n]);
    }// for 
}// downcase


int
main(int argc, char *argv[]) {

    char name[MAX_PATH], root[MAX_PATH];
    getNameAndRoot(name, root);

    // Set the environment root
    char rootenv[MAX_PATH + 30];
    snprintf(rootenv, sizeof(rootenv), "RELARN_INSTALL_ROOT=%s", root);
    _putenv(rootenv);

    // Assemble the path to the relarn executable
    char exepath[MAX_PATH];
    snprintf(exepath, sizeof(exepath), "%s\\lib\\relarn\\relarn.exe", root);

    downcase(name);
    if (strcmp(name, "relarn.exe") == 0) {
        _execl(exepath, exepath, NULL);
    } else if (strcmp(name, "relarn-scores.exe") == 0) {
        _execl(exepath, exepath, "-s", NULL);
    } else if (strcmp(name, "relarn-winning-scores.exe") == 0) {
        _execl(exepath, exepath, "-i", NULL);
    } else {
        MessageBoxA(NULL,
                    "You don't seem to be affected.",
                    "Err0r", MB_OK | MB_ICONERROR);
    }// if .. else

    
    return 0;
}
