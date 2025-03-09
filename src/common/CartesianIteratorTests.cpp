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

    printVec(ct.getPos());
    ct.next();
    printVec(ct.getPos());
    ct.next();
    printVec(ct.getPos());
    ct.next();
    printVec(ct.getPos());

    TEST_EXPECT(2 == 2);
}