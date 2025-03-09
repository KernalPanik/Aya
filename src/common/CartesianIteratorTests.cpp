#include "CartesianIteratorTests.h"
#include "CartesianIterator.h"
#include "../../test/Framework/testRunnerUtils.h"

#include <iostream>
#include <vector>

void printVec(const std::vector<size_t>& vec) {
    for (const unsigned long i : vec) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

void CartesianIterator_IteratesProperly() {
    std::vector<size_t> arraySizes = {4, 2, 3};
    auto ct = CartesianIterator(arraySizes);

    for (size_t i = 0; i < 3; i++) {
        ct.next();
        printVec(ct.getPos());
    }
    const std::vector<size_t> testVec = {0, 1, 0};
    TEST_EXPECT(ct.getPos() == testVec);

    for (size_t i = 0; i < 3; i++) {
        ct.next();
    }

    const std::vector<size_t> testVec1 = {1, 0, 0};
    TEST_EXPECT(ct.getPos() == testVec1);
}

void CartesianIterator_ReachesEnd() {
    std::vector<size_t> arraySizes = {4, 2, 3};
    auto ct = CartesianIterator(arraySizes);
    TEST_EXPECT(!ct.isDone());
    while (!ct.isDone()) {
        ct.next();
    }
    TEST_EXPECT(ct.isDone());
}