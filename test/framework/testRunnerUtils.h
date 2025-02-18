#pragma once

#include <stdio.h>
#include <stdexcept>

static inline int AssertThrow(const char *expect, const char* file, const char* func, int line) {
    fprintf(stderr,"%s:%d %s: expectation (%s) failed.\n", file , line , func , expect);
    throw std::logic_error("test");
}

#define TEST_EXPECT(condition) (condition ? : AssertThrow(#condition, __FILE__, __func__, __LINE__))