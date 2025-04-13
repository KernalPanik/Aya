#pragma once

#include <chrono>

class TestRunner {
public:
    explicit TestRunner(bool failOnAssert);

    void SetFailOnAssert(bool value);

    void RunTest(void (test)(), const char *printableFuncName);

private:
    bool failOnAssert;
    size_t totalTestCount;
    size_t failureCount;

    std::chrono::microseconds totalExecutionTimeMs;
};
