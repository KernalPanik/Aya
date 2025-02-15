#include "util-tests.h"
#include "util.h"
#include "../../test/framework/testRunnerUtils.h"
#include "tuple-utils.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <numeric>
#include <iostream>

void UtilTests_memswap_chars()
{
    char x = 'x';
    char y = 'y';

    memswap((ptr)&x, (ptr)&y, sizeof(char));

    TEST_EXPECT(x == 'y');
    TEST_EXPECT(y == 'x');
}

void UtilTests_memswap_mallocdMem()
{
    size_t dataSize = 100;

    char* largeString1 = (char*)malloc(dataSize);
    char* largeString2 = (char*)malloc(dataSize);

    strcpy(largeString1, "TEST ABC DEF GHI");
    strcpy(largeString2, "QWEQ KEK LOL");

    memswap((ptr)largeString1, (ptr)largeString2, dataSize);

    TEST_EXPECT(strcmp(largeString1, "QWEQ KEK LOL") == 0);
    TEST_EXPECT(strcmp(largeString2, "TEST ABC DEF GHI") == 0);

    free(largeString1);
    free(largeString2);
}

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
    std::string wantedOutput = std::string("[testString; 20; 42; ]");
    std::string s = TupleToString(t);
    TEST_EXPECT(s == wantedOutput);
}

void UtilTests_Tuple_SimpleComparison(){}