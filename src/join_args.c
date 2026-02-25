#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "errors.h"

#define HEAP_BASE_SIZE 8196
#define GROWTH_FACTOR 2

char* join_args(int32_t argc, char** argv) {
    if (argc <= 1) {
        return (char*) NULL;
    }

    size_t heap_size = 0;
    size_t pos = 0;
    char* heap_base = NULL;

    for (size_t i = 1; i < (size_t) argc; i++) {
        size_t len = strlen(argv[i]);
        size_t need = pos + len + 1;

        while (need > heap_size) {
            heap_size = (heap_size == 0) ? HEAP_BASE_SIZE : heap_size * GROWTH_FACTOR;
            char* new_heap_base = (char*) realloc(heap_base, heap_size);
            if (new_heap_base == NULL) {
                fprintf(stderr, "Error: Could not allocate %d bytes of memory!", (int) heap_size);
                free(heap_base);
                exit(REALLOC_FAIL_EC);
            }

            heap_base = new_heap_base;
        }

        memcpy((heap_base + pos), argv[i], len);
        pos += len;
        heap_base[pos++] = ' ';
    }

    if (heap_base != NULL) {
        heap_base[pos - 1] = '\0';
    }

    return heap_base;
}

void join_args_free(char* heap) {
    if (heap == NULL) {
        return;
    }

    free(heap);
}
