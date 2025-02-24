#include "testableFunction-tests.h"
#include "testableFunction.h"
#include "../../test/framework/testRunnerUtils.h"
#include "../common/tuple-utils.h"

#include <iostream>
#include <functional>
#include <string>
#include <tuple>
#include <utility>

static int nonStateChangingNonVoidFunc(std::string& s, int t) {
    return 42;
}

static short StateChangingNonVoidFunc(float& t) {
    t += 0.8f;
    return 42;
}

static void nonStateChangingVoidFunc(int x) {
    return;
}

static void StateChangingVoidFunc(double& d) {
    d += 24;
}

void TestableFunction_SimpleReturningFunction() {
    auto testableFunction = ConstructTestableFunction<int, std::string&, int&>(nonStateChangingNonVoidFunc);
    auto packedInputs = std::make_tuple(std::string("Test"), 12);
    auto expectedState = std::make_tuple(42, std::string("Test"), 12);
    auto finalState = InvokeWithPackedArguments<int>(testableFunction, std::move(packedInputs));

    //NOTE: This call crashes the compiler (Apple Clang 16):
    //auto finalState = InvokeTestableFunction<int, std::string&, int&>(testableFunction, t, 12);
    // TODO: move to separate task

    TEST_EXPECT(expectedState == finalState);
}

void TestableFunction_NonVoidStateChanging_StateChanged() {
    auto testableFunction = ConstructTestableFunction<short, float&>(StateChangingNonVoidFunc);
    auto packedInputs = std::make_tuple(41.2f);
    auto expectedState = std::make_tuple(42, 42.f);
    auto finalState = InvokeWithPackedArguments<short>(testableFunction, std::move(packedInputs));

    TEST_EXPECT(expectedState == finalState);
}

void TestableFunction_VoidStateChanging_StateChanged() {
    auto testableFunction = ConstructTestableFunction<void, double&>(StateChangingVoidFunc);
    auto packedInputs = std::make_tuple(static_cast<double>(10.5f));
    auto expectedState = std::make_tuple(static_cast<double>(34.5f));
    auto finalState = InvokeWithPackedArguments<void>(testableFunction, std::move(packedInputs));
    
    TEST_EXPECT(expectedState == finalState);
}

void TestableFunction_VoidNonStateChanging_StateUnchanged() {
    auto testableFunction = ConstructTestableFunction<void, int&>(nonStateChangingVoidFunc);
    auto packedInputs = std::make_tuple(static_cast<int>(42));
    auto expectedState = std::make_tuple(static_cast<int>(42));
    auto finalState = InvokeWithPackedArguments<void>(testableFunction, std::move(packedInputs));

    TEST_EXPECT(expectedState == finalState);
}