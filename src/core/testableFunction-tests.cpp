#include "testableFunction-tests.h"
#include "testableFunction.h"

#include <iostream>
#include <functional>
#include <string>
#include <tuple>
#include <utility>

int test(std::string s, double&& k) {
    std::cout << s << " " << k << std::endl;
    k += 1.0f;
    return 42;
}

void TestSimpleVoidFunction() {
    TestableFunction<int, std::string, double&&> func(test);
    auto x = func.Invoke("test33", 3.0f);

    std::cout << std::move(std::get<double&&>(x)) << std::endl;
    std::cout << std::move(std::get<int>(x)) << std::endl;
}

void TestSimpleFunctionWithReturn() {}

void TestSimpleVoidFunctionWithArgs() {}

void TestSimpleFunctionWithReturnWithArgs() {}