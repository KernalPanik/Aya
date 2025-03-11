#include "metamorphicRelation-tests.h"
#include "../../test/Framework/testRunnerUtils.h"
#include "src/Common/tuple-utils.h"
#include "src/Common/CartesianIterator.h"
#include "Modules/Transformer/transformer.h"
#include "Modules/MRGenerator/TestContext.hpp"
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
overload to use. It's easy to mitigate via slim wrappers like poww, for example.
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
    const size_t targetOutputStateIndex = 0;
    const size_t argCount = 2;
#pragma region Double Transformers
    std::vector<std::vector<double>> transformerArgumentPool;
    std::vector<std::function<void(double&, double)>> funcs = {Div, Sub, Mul, Add, Noop};
    std::vector<std::function<void(double&, double)>> funcsForOutput = {Div, Sub, Mul, Add};
    //std::vector<std::function<void(double&, double)>> funcs = {Add};

    transformerArgumentPool.push_back({1.0, 2.0, -1.0}); // for div function
    transformerArgumentPool.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool.push_back({0.0});

    std::vector<std::shared_ptr<ITransformer>> doubleTransformers;
    for (size_t i = 0; i < transformerArgumentPool.size(); i++) {
        auto t = TransformBuilder(funcs[i], transformerArgumentPool[i]).GetTransformers();
        doubleTransformers.insert(doubleTransformers.end(), t.begin(), t.end());
    }

    std::vector<std::shared_ptr<ITransformer>> doubleTransformersForOutput;
    for (size_t i = 0; i < transformerArgumentPool.size()-1; i++) {
        auto t = TransformBuilder(funcs[i], transformerArgumentPool[i]).GetTransformers();
        doubleTransformersForOutput.insert(doubleTransformersForOutput.end(), t.begin(), t.end());
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
    outputTransformerPool.insert({0, doubleTransformersForOutput});
    std::vector<size_t> outputTransformerCounts;
    outputTransformerCounts.reserve(outputTransformerCounts.size());
    for (auto &[fst, snd] : outputTransformerPool) {
        outputTransformerCounts.emplace_back(snd.size());
    }
    const std::vector outputTransformerIterators(outputTransformChainLength, CartesianIterator(outputTransformerCounts));
#pragma endregion

    std::vector<std::shared_ptr<ITestContext>> goodContexts; // Validation returned true for them -- potential MRs
#pragma region output mapping
    auto outputIterator = CompositeCartesianIterator(outputTransformerIterators);
    std::vector<std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>> outputTransformerChains;
    while (!outputIterator.isDone()) {
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformerChain;
        auto pos = outputIterator.getPos();
        for (auto &p : pos) {
            for (auto &i : p) {
                auto pair = std::make_shared<std::pair<size_t, std::shared_ptr<ITransformer>>>(std::make_pair(targetOutputStateIndex, outputTransformerPool[targetOutputStateIndex][i]));
                outputTransformerChain.push_back(pair);
            }
        }
        outputTransformerChains.push_back(outputTransformerChain);
        outputTransformerChain.clear();
        outputIterator.next();
    }
#pragma endregion
    auto compositeInputIterator = CompositeCartesianIterator(inputTransformerIterators);
    size_t overallMatchCount = 0;
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
        std::vector baseInputs = {10.0, 11.0, 12.0, 13.0};
        std::vector expInputs = {2.0, 3.0, 4.0};
        std::vector<std::any> inputs = {baseInputs, expInputs};
        std::vector<size_t> matchingOutputIndices = {0, 1}; // double pow(double double) => 0th and 1st indices match.
        for (auto &otc : outputTransformerChains) {
            auto ctx = TestContext<double, double, double>(poww, inputTransformerChain, otc, funcsForOutput, matchingOutputIndices);
            for (auto &bi : baseInputs) {
                for (auto &ei : expInputs) {
                    ctx.ValidateTransformChains({bi, ei}, 0);
                    if (ctx.GetTotalMatches() != 0) {
                        overallMatchCount += ctx.GetTotalMatches();
                    }
                }
            }
        }
        compositeInputIterator.next();
    }

    std::cout << "Found " << overallMatchCount << " possible MRs" << std::endl;

#pragma region TransformerPrep


#pragma endregion
    TEST_EXPECT(2 == 2); // TODO: After API is settled and MRs are produced, update this check to match MR count for Pow()
}