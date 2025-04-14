#include "testRunner.h"

#include <string>
#include <iostream>
#include <stdexcept>

static void PrintTestFailureReport(const char *testName, std::chrono::microseconds ms) {
    std::cout << testName << " \t[\e[1;91mFAIL\e[0m]" << "[ " << ms.count() << " microseconds]" << std::endl;
}

static void PrintTestPassReport(const char *testName, std::chrono::microseconds ms) {
    std::cout << testName << " \t[\e[1;92mOK\e[0m]" << "[ " << ms.count() << " microseconds]" << std::endl;
}

TestRunner::TestRunner(bool failOnAssert) {
    this->totalTestCount = 0;
    this->failureCount = 0;
    this->failOnAssert = failOnAssert;
    this->totalExecutionTimeMs = std::chrono::microseconds(0);
}

void TestRunner::SetFailOnAssert(bool value) {
    this->failOnAssert = value;
}

void TestRunner::RunTest(void (test)(), const char *printableFuncName) {
    std::chrono::microseconds ms;
    auto start = std::chrono::high_resolution_clock::now();

    try {
        totalTestCount++;
        test();
        auto end = std::chrono::high_resolution_clock::now();
        ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    } catch (std::logic_error &e) {
        PrintTestFailureReport(printableFuncName, ms);
        failureCount++;

        auto end = std::chrono::high_resolution_clock::now();
        ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        if (failOnAssert) {
            exit(1);
        }

        return;
    }

    PrintTestPassReport(printableFuncName, ms);
}
