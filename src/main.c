#include <stdio.h>

#include "join_args.h"
#include "usage.h"
#include "api.h"

int main(int argc, char** argv) {
    char* arg_heap = join_args((int32_t) argc, argv);
    if (arg_heap == NULL) {
        usage(argv[0]);
    }

    claude_get(arg_heap);

    join_args_free(arg_heap);
    return 0;
}
