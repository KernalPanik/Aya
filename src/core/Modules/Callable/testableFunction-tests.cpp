#include "testableFunction-tests.h"
#include "testableFunction.h"
#include "test/Framework/testRunnerUtils.h"
#include "TestableFunction.hpp"

#include <iostream>
#include <string>
#include <tuple>

//using namespace Callable;

#pragma region Helper Functions
static int nonStateChangingNonVoidFunc(std::string& s, int t) {
    return 42;
}

static short StateChangingNonVoidFunc(float& t) {
    t += 0.8f;
    return 42;
}

static void nonStateChangingVoidFunc(int x) {}

static void StateChangingVoidFunc(double& d) {
    d += 24;
}
#pragma endregion

void TestableFunction_SimpleReturningFunction() {
    std::vector<float> inputs = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    std::vector<std::shared_ptr<ITestableFunction>> tests;

    float x = 5;
    auto func = std::make_shared<TestableFunction<short, float&>>(StateChangingNonVoidFunc, x);

    for (auto &i : inputs) {
        tests.push_back(std::make_shared<TestableFunction<short, float&>>(StateChangingNonVoidFunc, i));
    }

    for (auto &s : tests) {
        s->Invoke();
    }

    for (auto &i : tests) {
        i->PrintState();
    }

    for (auto &inp : inputs) {
        std::cout << inp << std::endl;
    }
}

    /*
void TestableFunction_NonVoidStateChanging_StateChanged() {
    const auto testableFunction = ConstructTestableFunction<short, float&>(StateChangingNonVoidFunc);
    auto packedInputs = std::make_tuple(41.2f);
    const auto expectedState = std::make_tuple(42, 42.f);
    const auto finalState = InvokeWithPackedArguments<short>(testableFunction, std::move(packedInputs));

    TEST_EXPECT(expectedState == finalState);
}

void TestableFunction_VoidStateChanging_StateChanged() {
    const auto testableFunction = ConstructTestableFunction<void, double&>(StateChangingVoidFunc);
    auto packedInputs = std::make_tuple(static_cast<double>(10.5f));
    const auto expectedState = std::make_tuple(static_cast<double>(34.5f));
    const auto finalState = InvokeWithPackedArguments<void>(testableFunction, std::move(packedInputs));
    
    TEST_EXPECT(expectedState == finalState);
}

void TestableFunction_VoidNonStateChanging_StateUnchanged() {
    const auto testableFunction = ConstructTestableFunction<void, int&>(nonStateChangingVoidFunc);
    auto packedInputs = std::make_tuple(static_cast<int>(42));
    const auto expectedState = std::make_tuple(static_cast<int>(42));
    const auto finalState = InvokeWithPackedArguments<void>(testableFunction, std::move(packedInputs));

    TEST_EXPECT(expectedState == finalState);
}
*/