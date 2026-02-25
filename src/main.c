#include <stdio.h>

#include "join_args.h"
#include "usage.h"

int main(int argc, char** argv) {
    char* arg_heap = join_args((int32_t) argc, argv);
    if (arg_heap == NULL) {
        return 2;
    }
    printf("%s", arg_heap);
    join_args_free(arg_heap);

    return 0;
}
