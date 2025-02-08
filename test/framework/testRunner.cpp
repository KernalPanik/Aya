#include "testRunner.h"

#include <string>
#include <iostream>
#include <stdexcept>

static void PrintTestFailureReport(const char* testName) {
    printf("%s [\e[1;91mFAIL\e[0m]\n", testName);
}

static void PrintTestPassReport(const char* testName) {
    printf("%s [\e[1;92mOK\e[0m]\n", testName);
}

TestRunner::TestRunner(bool failOnAssert) {
    this->failOnAssert = failOnAssert;
}

void TestRunner::SetFailOnAssert(bool value) {
    this->failOnAssert = value;
}

void TestRunner::RunTest(void(test)(), const char* printableFuncName) {
    try {
        test();
    } catch (std::logic_error &e) {
        PrintTestFailureReport(printableFuncName);

        if (failOnAssert) {
            exit(1);
        }

        return;
    }

    PrintTestPassReport(printableFuncName);
}