#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "join_args.h"
#include "read_stdin.h"

#include "errors.h"
#include "usage.h"
#include "api.h"

static char* join_prompts(char* args, char* in) {
    if (args == NULL && in == NULL) {
        return NULL;
    } else if (args == NULL) {
        return in;
    } else if (in == NULL) {
        return args;
    }

    size_t args_len = strlen(args);
    size_t sin_len = strlen(in);

    size_t need = args_len + 2 + sin_len + 1;
    char* out = malloc(need);
    if (out == NULL) {   
        join_args_free(args);
        read_stdin_free(in);
        FATAL(MALLOC_FAIL_EC, "Error: Could not allocate %d bytes of memory", (int) need);
    }

    memcpy(out, args, args_len);
    out[args_len + 0] = '\n';
    out[args_len + 1] = '\n';
    memcpy(out + args_len + 2, in, sin_len);
    out[args_len + 2 + sin_len] = '\0';

    join_args_free(args);
    read_stdin_free(in);
    return out;
}

int main(int argc, char** argv) {
    char* arg_heap = join_args((int32_t) argc, argv);
    char* stdin_heap = NULL;
    if (!isatty(STDIN_FILENO)) {
        stdin_heap = read_stdin();
    }

    char* prompt = join_prompts(arg_heap, stdin_heap);
    if (prompt == NULL) {
        usage(argv[0]);
    }

    claude_get(prompt);

    free(prompt);
    return 0;
}
