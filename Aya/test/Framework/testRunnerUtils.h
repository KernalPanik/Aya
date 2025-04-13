#pragma once

#include <cstdio>
#include <stdexcept>

static int AssertThrow(const char *expect, const char *file, const char *func, int line) {
    fprintf(stderr, "%s:%d %s: expectation (%s) failed.\n", file, line, func, expect);
    throw std::logic_error("test");
}

static int noop() { return 0; };

#define TEST_EXPECT(condition) (condition ? noop() : AssertThrow(#condition, __FILE__, __func__, __LINE__))
