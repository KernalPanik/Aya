#pragma once

#include "testableFunction.h"

void TestableFunction_SimpleReturningFunction();
void TestableFunction_VoidNonStateChanging_StateUnchanged();
void TestableFunction_NonVoidStateChanging_StateChanged();
void TestableFunction_VoidStateChanging_StateChanged();
void TestableFunction_VoidNonStateChanging_StateUnchanged();