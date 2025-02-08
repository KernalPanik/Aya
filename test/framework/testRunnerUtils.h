#pragma once

#include <stdio.h>
#include <stdexcept>

inline int AssertThrow(const char *expect, const char* file, const char* func, int line) {
    fprintf(stderr,"%s:%d %s: expectation (%s) failed.\n", file , line , func , expect);
    throw std::logic_error("test");
}