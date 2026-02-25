#ifndef HOW_ERRORS_H
#define HOW_ERRORS_H

#include <stdio.h>
#include <stdlib.h>

#define REALLOC_FAIL_EC 1

#define FATAL(error_code, ...) \
    fprintf(stderr, __VA_ARGS__); \
    exit(error_code);

#endif // HOW_ERRORS_H
