#include "framework/testRunner.h"

#include <iostream>
#include <assert.h>

// TEST HEADERS
#include "framework/testRunnerTests.h"
#include "../src/common/util-tests.h"
#include "../src/core/testableFunction-tests.h"
// TEST HEADERS END

//NOTE: Pass '1' to AyaTest to stop at first assert failure
int main(int argc, char** argv)
{
    TestRunner tr(false);
    if (argc == 2 && atoi(argv[1]) == 1)
    {
        std::cout << "Will stop at first assert failure." << std::endl;
        tr.SetFailOnAssert(true);
    }
    
    tr.RunTest(TestRunnerSanityCheck, "TestRunnerSanityCheck");
    tr.RunTest(UtilTests_memswap_chars, "UtilTests_memswap_chars");
    tr.RunTest(UtilTests_memswap_mallocdMem, "UtilTests_memswap_mallocd");
    tr.RunTest(UtilTests_shuffle_doesnt_produce_garbage, "UtilTests_shuffle_doesnt_produce_garbage");
    tr.RunTest(UtilTests_Tuple_SimpleToString, "UtilTests_Tuple_SimpleToString");
    tr.RunTest(UtilTests_Tuple_SimpleComparison, "UtilTests_Tuple_SimpleComparison");

    // Testable function tests
    tr.RunTest(TestSimpleVoidFunction, "TestSimpleVoidFunction");
    tr.RunTest(TestSimpleFunctionWithReturn, "TestSimpleFunctionWithReturn");
    tr.RunTest(TestSimpleVoidFunctionWithArgs, "TestSimpleVoidFunctionWithArgs");
    tr.RunTest(TestSimpleFunctionWithReturnWithArgs, "TestSimpleFunctionWithReturnWithArgs");

    return 0;
}