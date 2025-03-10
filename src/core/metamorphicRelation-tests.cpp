#include "metamorphicRelation-tests.h"
#include "../../test/Framework/testRunnerUtils.h"
#include "src/Common/tuple-utils.h"
#include "src/Common/CartesianIterator.h"
#include "mrSearch.h"
#include "Modules/Callable/transformer.h"
#include "src/Common/util.hpp"

#include <iostream>
#include <map>
#include <cmath>
#include <tuple>
#include <cxxabi.h>
#include <src/Common/CompositeCartesianIterator.h>

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

void Noop(double& b, double val) {}

#pragma endregion

#pragma region transformers

#pragma endregion
//TODO: generate a warning when there is more than a million iterations to go through, suggest lowering potential transformer counts.

// Straightforward Generation of MRs without using MR generation function
void MR_SimpleConstructionTest() {
    // MR: pow(x, y + 1) == pow(x, y) * x

    // Tracked variable is the output. In final arg state is the element at index 0
    // Producing transformers for the output:

    const size_t outputTransformChainLength = 1;
    const size_t inputTransformChainLength = 1;
    const size_t argCount = 2;
#pragma region Double Transformers
    std::vector<std::vector<double>> transformerArgumentPool;
    std::vector<std::function<void(double&, double)>> funcs = {Div, Sub, Mul, Add, Noop};

    transformerArgumentPool.push_back({1.0, 2.0, -1.0, -2.0}); // for div function
    transformerArgumentPool.push_back({0.0, 1.0, 2.0, -1.0, -2.0});
    transformerArgumentPool.push_back({0.0, 1.0, 2.0, -1.0, -2.0});
    transformerArgumentPool.push_back({0.0, 1.0, 2.0, -1.0, -2.0});
    transformerArgumentPool.push_back({0.0});

    std::vector<std::shared_ptr<ITransformer>> doubleTransformers;
    for (size_t i = 0; i < transformerArgumentPool.size(); i++) {
        auto t = TransformBuilder(funcs[i], transformerArgumentPool[i]).GetTransformers();
        doubleTransformers.insert(doubleTransformers.end(), t.begin(), t.end());
    }
#pragma endregion

#pragma region CartesianIterator builders
    // Map index to possible input Transformers by type
    std::map<size_t, std::vector<std::shared_ptr<ITransformer>>> inputTransformerPool;
    inputTransformerPool.insert({0, doubleTransformers});
    inputTransformerPool.insert({1, doubleTransformers});

    // Create Cartesian Input Iterator to go through possible combinations
    std::vector<size_t> inputTransformerCounts;
    inputTransformerCounts.reserve(inputTransformerPool.size());
    for (auto &[fst, snd] : inputTransformerPool) {
        inputTransformerCounts.emplace_back(snd.size());
    }
    const std::vector inputTransformerIterators(inputTransformChainLength, CartesianIterator(inputTransformerCounts));

    // TODO: Move to separate func:
    std::map<size_t, std::vector<std::shared_ptr<ITransformer>>> outputTransformerPool;
    outputTransformerPool.insert({0, doubleTransformers});
    std::vector<size_t> outputTransformerCounts;
    outputTransformerCounts.reserve(outputTransformerCounts.size());
    for (auto &[fst, snd] : outputTransformerPool) {
        outputTransformerCounts.emplace_back(snd.size());
    }
    const std::vector outputTransformerIterators(outputTransformChainLength, CartesianIterator(outputTransformerCounts));
#pragma endregion

    std::vector<std::shared_ptr<ITestContext>> goodContexts; // Validation returned true for them -- potential MRs

    auto compositeInputIterator = CompositeCartesianIterator(inputTransformerIterators);
    while (!compositeInputIterator.isDone()) {
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> inputTransformerChain;

        auto pos = compositeInputIterator.getPos();
        size_t index = 0;
        for (auto &p : pos) {
            for (auto &i : p) {
                auto pair = std::make_shared<std::pair<size_t, std::shared_ptr<ITransformer>>>(std::make_pair(index, inputTransformerPool[index][i]));
                inputTransformerChain.push_back(pair);
                index++;
            }
            index = 0;
        }

        // Map Output transformers here
        auto outputIterator = CompositeCartesianIterator(outputTransformerIterators);
        while (!outputIterator.isDone()) {
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformerChain;
            auto opos = outputIterator.getPos();
            index = 0;
            for (auto &p : opos) {
                for (auto &i : p) {
                    auto pair = std::make_shared<std::pair<size_t, std::shared_ptr<ITransformer>>>(std::make_pair(index, inputTransformerPool[index][i]));
                    outputTransformerChain.push_back(pair);
                    index++;
                }
                index = 0;
            }

            // build test context and validate it




            outputIterator.next();
        }

        compositeInputIterator.next();
    }
    /*
    for (size_t i = 0; i < inputTransformChainLength; i++) {
        auto iter = CartesianIterator(transformerCounts);
        while (!iter.isDone()) {
            auto pos = iter.getPos();
            for (const auto &p : pos) {
                //std::cout << p << " ";
            }
            //std::cout << std::endl;
            iter.next();
        }
    }*/

    /*std::vector<std::shared_ptr<std::pair<size_t, std::vector<std::shared_ptr<ITransformer>>>>> outputTransformers;
    const std::vector outputTransformerArgs = {1.0, 2.0, -1.0, -2.0};

    for (const auto& func : funcs) {
        auto funcPool = TransformBuilder(func, outputTransformerArgs).MapTransformersToStateIndex(0);
    }

    std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> inputTransformers;
    std::vector<std::vector<double>> inputTransformArgPool;
    inputTransformArgPool.push_back({1.0, 2.0, -1.0, -2.0});
    inputTransformArgPool.push_back({0.0, 1.0, 2.0, -1.0, -2.0});
    inputTransformArgPool.push_back({0.0, 1.0, 2.0, -1.0, -2.0});
    inputTransformArgPool.push_back({0.0, 1.0, 2.0, -1.0, -2.0});

    std::vector<ITestContext> contexts;

    for (size_t i = 0; i < inputTransformArgPool.size(); i++) {
        for (size_t index = 0; index < 2; index++) {
            auto funcPool = TransformBuilder(funcs[i], inputTransformArgPool[i]).MapTransformersToStateIndex(index);
            inputTransformers.insert(inputTransformers.end(), funcPool.begin(), funcPool.end());
            auto ctx = TestContext<double, double, double>(poww, inputTransformers, outputTransformers);
            contexts.push_back(ctx);
        }
    }
    std::vector baseInputs = {10.0, 11.0, 12.0, 13.0};
    std::vector expInputs = {2.0, 3.0, 4.0};
*/
    //TODO: helper func to concat function args into vectors of std::any

    /*auto outputDivPool = TransformBuilder<double, double>(Div, outputTransformerArgs).MapTransformersToStateIndex(targetOutputIndex);
    auto outputMulPool = TransformBuilder<double, double>(Mul, outputTransformerArgs).MapTransformersToStateIndex(targetOutputIndex);

    outputTransformers.insert(outputTransformers.end(), outputDivPool.begin(), outputDivPool.end());

    std::vector baseInputs = {10.0, 11.0, 12.0, 13.0};
    std::vector expInputs = {2.0, 3.0, 4.0};
    for (auto &b : baseInputs) {
        for (auto &e : expInputs) {
            contexts.push_back(std::make_shared<TestContext<double, double, double>>(poww, b, e, 0, outputTransformers));
        }
    }

    for (auto &ctx : contexts) {
        ctx->PrintState();
        ctx->TestInvoke();
        ctx->PrintState();
        std::cout << std::endl;
    }*/

#pragma region TransformerPrep

    /*const std::vector<double> transformArgsForDiv = {1.0f, 2.0f, -1.0f};

    // TODO: TransformPool abstraction
    auto DivPool = TransformBuilder<double, double>(Div, transformArgsForDiv);
    auto divTransforms = DivPool.MapTransformersToStateIndex(0);
    auto AddPool = TransformBuilder<double, double>(Add, transformArgsForDiv);
    auto addTransforms = AddPool.MapTransformersToStateIndex(1);

    for (auto &ctx : contexts) {
        for (auto &t : addTransforms) {
            auto x = std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>();
            x.push_back(t);
            ctx->ValidateTransformChains(x);
        }
        std::cout << std::endl;
    }*/

#pragma endregion

#pragma region MRGen

#pragma endregion
    TEST_EXPECT(2 == 2);
}