#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* malloc(size_t rawSize) {
    // add the length of the ptr immediately before the ptr
    size_t size = rawSize + sizeof(size_t);
    int page = getpagesize();
    // gap between start of page and start of chunk to return
    int offset = page - (size % page);
    int numGivenPages = 1 + (size - 1) / page;
    int numTotalPages = numGivenPages + 1;

    void* allMemory = mmap(NULL, numTotalPages * page, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (!allMemory) {
        return NULL;
    }

    void* deadPage = allMemory + (page * numGivenPages);
    mprotect(deadPage, page, PROT_NONE);

    void* startOfMem = allMemory + offset;
    *((size_t*) startOfMem) = rawSize;

    void* output = startOfMem + sizeof(size_t);

    return output;
}

void* calloc(size_t size1, size_t size2) {
    void* output = malloc(size1 * size2);
    if (!output) {
        return NULL;
    }
    memset(output, 0, size1 * size2);
    return output;
}

void* realloc(void* start, size_t size) {
    if (!start) {
        return malloc(size);
    }

    void* startOfMem = start - sizeof(size_t);
    size_t rawSize = *((size_t*) startOfMem);

    void* output = malloc(size);
    if (!output) {
        return NULL;
    }
    memcpy(output, start, size > rawSize ? rawSize : size);
    free(start);

    return output;
}

void free(void* memory) {
    if (!memory) {
        return;
    }
    void* startOfMem = memory - sizeof(size_t);
    size_t rawSize = *((size_t*) startOfMem);
    size_t size = rawSize + sizeof(size_t);
    
    int page = getpagesize();
    int offset = size - (size % page);
    int numGivenPages = 1 + (size - 1 / page);
    int numTotalPages = numGivenPages + 1;

    munmap(startOfMem - offset, numTotalPages * page);
}
