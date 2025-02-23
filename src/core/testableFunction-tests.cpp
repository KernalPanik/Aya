#include "testableFunction-tests.h"
#include "testableFunction.h"
#include "../../test/framework/testRunnerUtils.h"
#include "../common/tuple-utils.h"

#include <iostream>
#include <functional>
#include <string>
#include <tuple>
#include <utility>

static int nonStateChangingNonVoidFunc(std::string& s, long long t) {
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

int square(int x) {
    return x * x;
}

void TestableFunction_SimpleReturningFunction() {
    auto testable = ConstructTestableFunction<int, int>(square);
    InvokeTestableFunction(testable, 25);

    

 //   auto func = ConstructTestableFunction<int, std::string&, long long>(nonStateChangingNonVoidFunc);// TestableFunctionV2<int, std::string&, long long>(nonStateChangingNonVoidFunc);
 //   std::string t("Test");
 //   InvokeTestableFunction(func, &t, 12);
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