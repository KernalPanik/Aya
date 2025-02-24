#include "framework/testRunner.h"

#include <iostream>
#include <assert.h>

// TEST HEADERS
#include "framework/testRunnerTests.h"
#include "../src/common/util-tests.h"
#include "../src/core/testableFunction-tests.h"
#include "../src/core/metamorphicRelation-tests.h"
#include "../src/core/transformer-tests.h"
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

    // Testable function tests
    tr.RunTest(TestableFunction_SimpleReturningFunction, "TestableFunction_SimpleReturningFunction");
    tr.RunTest(TestableFunction_NonVoidStateChanging_StateChanged, "TestableFunction_NonVoidStateChanging_StateChanged");
    tr.RunTest(TestableFunction_VoidStateChanging_StateChanged, "TestableFunction_VoidStateChanging_StateChanged");
    tr.RunTest(TestableFunction_VoidNonStateChanging_StateUnchanged, "TestableFunction_VoidNonStateChanging_StateUnchanged");

    // Metamorphic Relation Tests
    //tr.RunTest(MR_SimpleConstructionTest, "MR_SimpleConstructionTest");

    // Transformer tests
    tr.RunTest(TransformerTests_MutableCall, "TransformerTests_MutableCall");
    tr.RunTest(TransformerTests_MutableStruct, "TransformerTests_MutableStruct");
    tr.RunTest(TransformerTests_MultipleTransformers, "TransformerTests_MultipleTransformers");

    return 0;
}