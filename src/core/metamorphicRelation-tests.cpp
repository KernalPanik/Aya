#include "metamorphicRelation-tests.h"
#include "../../test/Framework/testRunnerUtils.h"
#include "src/Common/tuple-utils.h"
#include "mrSearch.h"

#include <iostream>
#include <cmath>
#include <tuple>
#include <cxxabi.h>

using namespace Callable;

/*
Why wrap pow() in poww()?
cmath pow() contains many overloads. std::function<> template within TestableFunctionBase cannot deduce which
overload to use. Although this is a design issue, It's easy to mitigate via slim wrappers like poww.
*/

#pragma region Helper functions
double poww(double x, double y) {
    return pow(x, y);
}
#pragma endregion

#pragma region tmpMRContext

struct MRContext {
    std::shared_ptr<TestableFunctionBase> func;
    std::vector<TupleWrapperBase> inputs;

    MRContext(std::shared_ptr<TestableFunctionBase> testable, std::vector<TupleWrapperBase> initialInputs) {
        this->func = std::move(testable);
        this->inputs = std::move(initialInputs);
    }
};

//TODO: MRContext builder/constructor
//TODO: Make MRSearch accept MRContext only
#pragma endregion

// Straightforward Generation of MRs without using MR generation function
void MR_SimpleConstructionTest() {
    auto func = ConstructTestableFunction<double, double&, double&>(poww);
    auto packedInput = TupleWrapper(static_cast<double>(12), static_cast<double>(2));
    std::vector<TupleWrapperBase> packedInputs;
    packedInputs.push_back(packedInput);

    auto ctx = MRContext(func, packedInputs);    
    //using fType = std::invoke_result_t<decltype(poww), double, double>;

    // MR context generator still needs to get function return type T...
    auto finalState = InvokeWithPackedArguments<double>(func, std::move(packedInput.tup));

    std::cout << TupleToString(packedInput.tup) << std::endl;
    std::cout << TupleToString(finalState) << std::endl;
    std::cout << TupleToString(packedInput.tup) << std::endl;

    TEST_EXPECT(2 == 2);
}
