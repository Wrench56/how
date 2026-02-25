#ifndef HOW_FREEARGS_H
#define HOW_FREEARGS_H

#include <stdint.h>

char* join_args(int32_t argc, char** argv);
void join_args_free(char* heap);

#endif // HOW_FREEARGS_H
