#include <stdio.h>
#include <stdlib.h>


void __attribute__((noreturn)) usage(char* appname) {
    printf("Usage: %s [QUESTION]\n", appname);
    exit(0);
}
