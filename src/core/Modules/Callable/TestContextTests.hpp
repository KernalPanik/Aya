#pragma once

#include "TestContextTests.hpp"

void TestableFunction_SimpleReturningFunction();
void TestableFunction_VoidNonStateChanging_StateUnchanged();
void TestableFunction_NonVoidStateChanging_StateChanged();
void TestableFunction_VoidStateChanging_StateChanged();