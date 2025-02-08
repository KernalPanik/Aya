#ifndef UTIL
#define UTIL

#include <stdio.h>

#include "aya-types.h"

void memswap(ptr a, ptr b, size_t n);
void shuffle(size_t n, size_t x, size_t* storage);
void printVersion();

#endif // UTIL