#include "TestContextTests.hpp"
#include "TestContext.hpp"
#include "test/Framework/testRunnerUtils.h"
#include "src/Common/tuple-utils.h"

#include <iostream>
#include <string>
#include <tuple>

using namespace Callable;

#pragma region Helper Functions
static int nonStateChangingNonVoidFunc(std::string& s, int t) {
    return 42;
}

static short StateChangingNonVoidFunc(float& t) {
    t += 1.0f;
    return 42;
}

static void nonStateChangingVoidFunc(int x) {}

static void StateChangingVoidFunc(double& d) {
    d += 24;
}
#pragma endregion

#pragma region Tests
void TestableFunction_SimpleReturningFunction() {
    std::vector<float> inputs = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<float> originalInputs = { 1.0, 2.0, 3.0, 4.0, 5.0 }; // To check if inputs are unchanged (they should not change)
    std::vector<std::tuple<short, float>> expectedStates = 
    { 
        std::make_tuple(42, 2.0f),
        std::make_tuple(42, 3.0f),
        std::make_tuple(42, 4.0f),
        std::make_tuple(42, 5.0f),
        std::make_tuple(42, 6.0f), 
    };

    std::vector<std::shared_ptr<ITestContext>> tests;
    for (auto &i : inputs) {
        tests.push_back(std::make_shared<TestContext<short, float&>>(StateChangingNonVoidFunc, i));
    }

    for (auto &s : tests) {
        s->TestInvoke();
    }

    for (size_t i = 0; i < 5; i++) {
        TEST_EXPECT(tests[i]->Equals(TupleToString(expectedStates[i])));
    }

    TEST_EXPECT(inputs == originalInputs);
}

void TestableFunction_NonVoidStateChanging_StateChanged() {
    auto packedInput = std::make_tuple(std::string("test2"), 32);
    auto expectedState = std::make_tuple(42, std::string("test2"), 32);
    std::shared_ptr<ITestContext> test = std::make_shared<TestContext<int, std::string&, int>>(nonStateChangingNonVoidFunc, std::get<0>(packedInput), std::get<1>(packedInput));
    
    test->TestInvoke();

    TEST_EXPECT(test->Equals(TupleToString(expectedState)));
}

void TestableFunction_VoidStateChanging_StateChanged() {
    double x = 10.0f;
    auto packedInput = std::make_tuple(x);
    auto expectedState = std::make_tuple((double)34.0f);
    const auto test = std::make_shared<TestContext<void, double&>>(StateChangingVoidFunc, std::get<0>(packedInput));

    test->TestInvoke();
    TEST_EXPECT(test->Equals(TupleToString(expectedState)));
}

void TestableFunction_VoidNonStateChanging_StateUnchanged() {
    int x = 10;
    auto packedInput = std::make_tuple(x);
    auto expectedState = std::make_tuple(x);
    const auto test = std::make_shared<TestContext<void, int>>(nonStateChangingVoidFunc, std::get<0>(packedInput));

    test->TestInvoke();
    TEST_EXPECT(test->Equals(TupleToString(expectedState)));
}
#pragma endregion