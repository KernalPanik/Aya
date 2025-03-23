#include "transformer-tests.h"
#include "src/public/transformer.hpp"
#include "src/public/TransformBuilder.hpp"
#include "../../../../test/Framework/testRunnerUtils.h"

#include <iostream>
#include <string>
#include <functional>
#include <src/core/IntegrationTests/TestUtils/TransformerFunctions.hpp>

using namespace Aya;

#pragma region helper_functions
struct TestStruct {
    int testValue {};
    std::string testString;
    friend std::ostream& operator<<(std::ostream& os, const TestStruct& testStruct) {
        os << "TestStruct(" << testStruct.testValue << ";" << testStruct.testString << ")";
        return os;
    }
};

static void ModifyBase(int& base) {
    base *= 2;
}

static void ModifyStruct1(TestStruct& s, int x, int y, TestStruct z) {
    s.testValue += 2;
    s.testString += "a";
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

void TransformerTests_CallWithArgs() {
    auto baseValue = 10.0f;
    const auto transformer = ConstructTransformer<float, float>(AddVal, "Add", 12.0f);
    transformer->Apply(&baseValue);
    TEST_EXPECT(baseValue == 22.0f);
}

void TransformerTests_MutableCall() {
    auto baseValue = 10;
    const auto transformer = ConstructTransformer<int>(ModifyBase, "Modify");
    transformer->Apply(&baseValue);
    TEST_EXPECT(baseValue == 20);
}

void TransformerTests_MutableStruct() {
    TestStruct sample;
    sample.testValue = 40;
    sample.testString = "test";

    const auto transformer = ConstructTransformer<TestStruct>(ModifyStruct, "Modify");
    transformer->Apply(&sample);
    TEST_EXPECT(sample.testValue == 42);
    TEST_EXPECT(sample.testString == "testa");
}

void TransformerTests_ToString() {
    TestStruct sample;
    sample.testValue = 40;
    sample.testString = "test";

    TestStruct sample2;
    sample2.testValue = 40;
    sample2.testString = std::string("test");

    const auto transformer = ConstructTransformer<TestStruct, int, int, TestStruct>(ModifyStruct1, "Modify", 1, 2, std::move(sample2));
    transformer->Apply(&sample);

    TEST_EXPECT("Modify( input[0], 1, 2, TestStruct(40;test), )" == transformer->ToString("input", 0));
}