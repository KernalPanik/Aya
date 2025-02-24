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
    t += 24.0f;
    return 42;
}

static void nonStateChangingVoidFunc(int x) {
    return;
}

static void StateChangingVoidFunc(double& d) {
    d += 24;
}

static float SimpleReturningFunction() {
    return 42.1f;
}

void print(const std::string& message) {
    std::cout << message << std::endl;
}

void TestableFunction_SimpleReturningFunction() {
    std::string t("Test");
    int vals = 12;
    auto testableFunction = ConstructTestableFunction<int, std::string&, int&>(nonStateChangingNonVoidFunc);

    auto expectedState = std::make_tuple(42, t, vals);

    //NOTE: This call crashes the compiler (Apple Clang 16):
    //auto finalState = InvokeTestableFunction<int, std::string&, int&>(testableFunction, t, 12);

    auto finalState = InvokeTestableFunction<int>(testableFunction, t, vals);
    auto finalStateDecayed = TupleDecay(*finalState);

    TEST_EXPECT(CompareTuples(expectedState, *finalState));
}

void TestableFunction_SimpleReturningFunction_OperateOnInputTuples() {
    // Construct arguments as a tuple (emulate MR process)
    // Pass Tuple elements as args...
}

/*void TestableFunction_NonVoidNonStateChanging_StateUnchanged() {
    auto func = TestableFunction<int, std::string&, long long>(nonStateChangingNonVoidFunc);
    auto expectedState = std::make_tuple(42, "test", 12);
    std::string str("test");
    auto newState = func.Invoke(str, 12);
    TEST_EXPECT(expectedState == newState);
}

void TestableFunction_NonVoidStateChanging_StateChanged() {
    auto func = TestableFunction<short, float&>(StateChangingNonVoidFunc);
    float val = 20;
    auto expectedState = std::make_tuple(42, 44.0f);
    auto state = func.Invoke(val);
    TEST_EXPECT(state == expectedState);
}

void TestableFunction_VoidStateChanging_StateChanged() {
    auto func = TestableFunction<void, double&>(StateChangingVoidFunc);
    double val = 20;
    auto expectedState = std::make_tuple(44.0f);
    auto state = func.Invoke(val);
    TEST_EXPECT(state == expectedState);
}

void TestableFunction_VoidNonStateChanging_StateUnchanged() {
    auto func = TestableFunction<void, int>(nonStateChangingVoidFunc);
    int val = 10;
    auto expectedState = std::make_tuple(val);
    auto state = func.Invoke(std::move(val));
    TEST_EXPECT(state == expectedState);
}*/