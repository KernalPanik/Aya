#include "util-tests.h"
#include "util.hpp"
#include "../../test/Framework/testRunnerUtils.h"
#include "tuple-utils.h"

#include <vector>
#include <numeric>
#include <iostream>

// Just shuffles numbers randomly. So as long as it doesn't crash, it's fine
void UtilTests_shuffle_doesnt_produce_garbage() {
    std::vector<size_t> indices(10);
    std::iota(std::begin(indices), std::end(indices), 0);

    const size_t wantedSize = 5;

    auto shuffledVec = shuffle(indices, wantedSize);
    for (size_t i = 0; i < wantedSize; i++) {
        std::cout << shuffledVec[i] << " ";
        TEST_EXPECT(shuffledVec[i] < 10);
    }
}

void UtilTests_Tuple_SimpleToString() {
    auto t = std::make_tuple("testString", 20.0f, 42);
    std::string wantedOutput = std::string("testString, 20, 42, ");
    std::string s = Aya::TupleToString(t);
    TEST_EXPECT(s == wantedOutput);
}

void UtilTests_Vector_Tuplify() {
    std::vector<std::any> v = {1, 2, 3.0f, std::string("3")};
    const std::tuple<int, int, float, std::string> w = std::make_tuple(1, 2, 3.0f, std::string("3"));
    const std::tuple<int, int, float, std::string> received = Aya::Tuplify<int, int, float, std::string>(v);
    TEST_EXPECT(w == received);
}
