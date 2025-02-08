#include "testRunnerTests.h"
#include "testRunnerUtils.h"

void TestRunnerSanityCheck() {
    int x = 2;
    TEST_EXPECT(x * 2 == 4);
}