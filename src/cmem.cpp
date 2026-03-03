#include "cmem.hpp"

#include <cstdlib>
#include <cstring>
#include <cstdio>

void* mreloc(const void* original, size_t size) {
    void* n = malloc(size);
    if (n == NULL) {
        fprintf(stderr, "chario: malloc() failed.\n");
        fflush(stderr);
        abort();
    }
    memcpy(n, original, size);
    return n;
}