#include "transformer-tests.h"
#include "transformer.hpp"
#include "TransformBuilder.hpp"
#include "../../../../test/Framework/testRunnerUtils.h"

#include <iostream>
#include <string>
#include <functional>

using namespace Aya;

#pragma region helper_functions
struct TestStruct {
    int testValue {};
    std::string testString;
};

static void ModifyBase(int& base) {
    base *= 2;
}

static void ModifyStruct(TestStruct& s) {
    s.testValue += 2;
    s.testString += "a";
}

static void Increment(float& f) {
    f++;
}

static void AddVal(float& f, float val) {
    f += val;
}
#pragma endregion

void TransformerTests_MutableCall() {
    auto baseValue = 10;
    const auto transformer = ConstructTransformer<int>(ModifyBase);
    transformer->Apply(&baseValue);
    TEST_EXPECT(baseValue == 20);
}

void TransformerTests_MutableStruct() {
    TestStruct sample;
    sample.testValue = 40;
    sample.testString = "test";

    const auto transformer = ConstructTransformer<TestStruct>(ModifyStruct);
    transformer->Apply(&sample);
    TEST_EXPECT(sample.testValue == 42);
    TEST_EXPECT(sample.testString == "testa");
}