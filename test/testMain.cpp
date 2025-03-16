#include "Framework/testRunner.h"

#include <iostream>
#include <cassert>

// TEST HEADERS
#include "../src/Common/util-tests.h"
#include "../src/Common/CartesianIteratorTests.h"
#include "../src/Core/Modules/Transformer/transformer-tests.h"
#include "src/core/IntegrationTests/MetamorphicRelationGenTests.hpp"
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
    
    tr.RunTest(UtilTests_shuffle_doesnt_produce_garbage, "UtilTests_shuffle_doesnt_produce_garbage");
    tr.RunTest(UtilTests_Tuple_SimpleToString, "UtilTests_Tuple_SimpleToString");

    // Cartesian Iterator Tests
    tr.RunTest(CartesianIterator_IteratesProperly, "CartesianIterator_IteratesProperly");
    tr.RunTest(CartesianIterator_ReachesEnd, "CartesianIterator_ReachesEnd");

    // Transformer tests
    tr.RunTest(TransformerTests_MutableCall, "TransformerTests_MutableCall");
    tr.RunTest(TransformerTests_MutableStruct, "TransformerTests_MutableStruct");

    // Metamorphic Relation Tests
    tr.RunTest(MetamorphicRelationGenTests::MetamorphicRelationTest_Pow, "MetamorphicRelationTest_Pow");
    tr.RunTest(MetamorphicRelationGenTests::MetamorphicRelationTest_VectorSize, "MetamorphicRelationTest_VectorSize");

    return 0;
}