#ifndef HOW_ERRORS_H
#define HOW_ERRORS_H

#include <stdio.h>
#include <stdlib.h>

#define REALLOC_FAIL_EC 1
#define SOCK_FAIL_EC 2
#define SOCK_NOT_FOUND_EC 3
#define SSL_CONNECT_FAIL_EC 4
#define MALLOC_FAIL_EC 5
#define SSL_SEND_FAIL_EC 6
#define SSL_RECV_FAIL_EC 7

#define FATAL(error_code, ...) \
    fprintf(stderr, __VA_ARGS__); \
    exit(error_code);

#endif // HOW_ERRORS_H
