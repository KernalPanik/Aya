#include "util.h"
#include "aya-types.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <numeric>

void memswap(ptr a, ptr b, size_t n)
{
    size_t chunkCount = n / sizeof(ptr);
    size_t leftover = n % sizeof(ptr);

    for (size_t i = 0; i < chunkCount; i++)
    {
        ptr tmp = 0;
        memcpy((void*)&tmp, (void*)(a + i * sizeof(ptr)), sizeof(ptr));
        memcpy((void*)(a + i * sizeof(ptr)), (void*)(b + i * sizeof(ptr)), sizeof(ptr));
        memcpy((void*)(b + i * sizeof(ptr)), &tmp, sizeof(ptr));
    }

    for (size_t i = n - leftover; i < n; i++)
    {
        char tmp = 0;
        memcpy((void*)&tmp, (void*)(a + i * sizeof(char)), sizeof(char));
        memcpy((void*)(a + i * sizeof(char)), (void*)(b + i * sizeof(char)), sizeof(char));
        memcpy((void*)(b + i * sizeof(char)), &tmp, sizeof(char));
    }
}

// https://en.wikipedia.org/wiki/Fisher–Yates_shuffle
/*
 *  Shuffles an array of ints and takes first N elements
 *  originalstorage -- vector to shuffle
 *  wantedSize -- shuffled vector size
 */
// TODO: make it templated, pass T instead of size_t
std::vector<size_t> shuffle(const std::vector<size_t>& originalStorage, const size_t wantedSize) {
    std::vector<size_t> indices(originalStorage.size());
    std::iota(std::begin(indices), std::end(indices), 0);

    for (size_t i = originalStorage.size() - 1; i > 0; i--) {
        srand(time(NULL));
        size_t j = rand() % (i + 1);
        size_t t = indices[i];
        indices[i] = indices[j];
        indices[j] = t;
    }

    std::vector<size_t> shuffledVector;
    for (size_t i = 0; i < wantedSize; i++) {
        shuffledVector.push_back(indices[i]);
    }

    return shuffledVector;
}

// Sometime in the future, grab version metadata from .git
void printVersion()
{
    printf("\nAya2 indev\n");
}