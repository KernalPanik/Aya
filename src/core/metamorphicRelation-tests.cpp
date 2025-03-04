#include "metamorphicRelation-tests.h"
#include "../../test/Framework/testRunnerUtils.h"
#include "src/Common/tuple-utils.h"
#include "mrSearch.h"
#include "Modules/Callable/transformer.h"

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

void Add(double& b, double val) {
    b += val;
}

void Mul(double& b, double val) {
    b *= val;
}

void Sub(double& b, double val) {
     b -= val;
}

void Div(double& b, double val) {
    b /= val;
}

#pragma endregion

#pragma region transformers

#pragma endregion

#pragma region tmpMRContext

/*
struct MRContext {
    std::shared_ptr<TestableFunctionBase> func;
    std::vector<TupleWrapperBase> inputs;

    MRContext(std::shared_ptr<TestableFunctionBase> testable, std::vector<TupleWrapperBase> initialInputs) {
        this->func = std::move(testable);
        this->inputs = std::move(initialInputs);
    }
};
*/
//TODO: MRContext builder/constructor
//TODO: Make MRSearch accept MRContext only
#pragma endregion

// Straightforward Generation of MRs without using MR generation function
void MR_SimpleConstructionTest() {
    // MR: pow(x, y+1) == pow(x, y) * x
    // TODO: template util for packed input creation
    // BuildPackedInputs(std::vector<T> args)

    std::vector<std::shared_ptr<ITestContext>> contexts;
    std::vector baseInputs = {static_cast<double>(10.0f), static_cast<double>(11.0f), static_cast<double>(12.0f), static_cast<double>(13.0f)};
    std::vector expInputs = {static_cast<double>(2.0f), static_cast<double>(3.0f), static_cast<double>(4.0f)};
    for (auto &b : baseInputs) {
        for (auto &e : expInputs) {
            contexts.push_back(std::make_shared<TestContext<double, double, double>>(poww, b, e));
        }
    }

    for (auto &ctx : contexts) {
        ctx->PrintState();
        ctx->TestInvoke();
        ctx->PrintState();
        std::cout << std::endl;
    }

#pragma region TransformerPrep

    std::vector<double> transformArgsForDiv = {1.0f, 2.0f, -1.0f};
    std::vector<double> transformArgsGeneric = {1.0f, 2.0f, -1.0f, 0.0f, -2.0f};

    auto DivPool = TransformPool<double, double>(Div, transformArgsForDiv);
    auto divTransforms = DivPool.GetTransformers(0);

    for (auto &ctx : contexts) {
        for (auto &t : divTransforms) {
            ctx->ValidateTransformChains(t);
        }
        std::cout << std::endl;
    }

#pragma endregion

#pragma region MRGen



#pragma endregion
    TEST_EXPECT(2 == 2);
}