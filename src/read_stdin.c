#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "errors.h"

#define HEAP_BASE_SIZE 8196
#define GROWTH_FACTOR 2

char* read_stdin(void) {
    size_t heap_size = 0;
    size_t pos = 0;
    char* heap_base = NULL;

    for (;;) {
        size_t need = pos + 1 + 1;
        while (need > heap_size) {
            heap_size = (heap_size == 0) ? HEAP_BASE_SIZE : heap_size * GROWTH_FACTOR;
            char* new_heap_base = (char*) realloc(heap_base, heap_size);
            if (new_heap_base == NULL) {
                free(heap_base);
                FATAL(REALLOC_FAIL_EC, "Error: Could not allocate %d bytes of memory!\n", (int) heap_size);
            }

            heap_base = new_heap_base;
        }

        ssize_t n = read(STDIN_FILENO, heap_base + pos, heap_size - pos - 1);
        if (n < 0) {
            free(heap_base);
            FATAL(IO_FAIL_EC, "Error: Could not read from stdin!\n");
        }

        if (n == 0) {
            break;
        }

        pos += (size_t) n;
    }

    if (heap_base == NULL) {
        return NULL;
    }

    heap_base[pos] = '\0';
    return heap_base;
}

void read_stdin_free(char* heap) {
    if (heap == NULL) {
        return;
    }

    free(heap);
}
