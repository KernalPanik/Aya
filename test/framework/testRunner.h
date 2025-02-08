#pragma once

#include "testRunnerUtils.h"

#define TEST_EXPECT(condition) (condition ? : AssertThrow(#condition, __FILE__, __func__, __LINE__))
#define TEST_EXPECT1(condition) (condition ? : fprintf(stderr,"%s:%d %s: expectation (%s) failed.\n", __FILE__ , __LINE__ , __func__ , #condition))

struct TestRunner 
{
    bool failOnAssert;

    TestRunner(bool failOnAssert);

    void SetFailOnAssert(bool value);
    void RunTest(void(test)(), const char* printableFuncName);
};