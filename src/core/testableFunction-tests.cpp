#include "testableFunction-tests.h"
#include "testableFunction.h"

#include <iostream>
#include <functional>
#include <string>

void test(int x) {
    std::cout << "Test1 " << x << std::endl;
}

void test2(std::string s, double k) {
    std::cout << s << " " << k << std::endl;
}

void TestSimpleVoidFunction() {
    std::function<void(int)> tx;
    tx = test;
    tx(2);

    TestableFunction<void, int> txx(test);
    txx.Invoke(2);    
}

void TestSimpleFunctionWithReturn() {}

void TestSimpleVoidFunctionWithArgs() {}

void TestSimpleFunctionWithReturnWithArgs() {}