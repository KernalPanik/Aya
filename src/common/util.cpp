#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

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
 *  n -- total amount of indices
 *  x -- max amount of indices wanted
 */
//TODO: rework to use vectors
void shuffle(size_t n, size_t x, size_t* storage) {
    if (storage == NULL) {
        printf("storage is NULL\n");
        return;
    }

    size_t* arr = (size_t*)malloc(sizeof(size_t) * n);

    for (size_t i = 0; i < n; i++) {
        arr[i] = i;
    }

    for (size_t i = n - 1; i > 0; i--) {
        srand(time(NULL));
        size_t j = rand() % (i + 1);
        size_t t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
    }

    for (size_t i = 0; i < x; i++) {
        storage[i] = arr[i];
    }

    free(arr);
}

// Sometime in the future, grab version metadata from .git
void printVersion()
{
    printf("\nAya2 indev\n");
}