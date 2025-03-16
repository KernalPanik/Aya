#pragma once

#include "test/Framework/testRunnerUtils.h"
#include "src/core/IntegrationTests/TestUtils/TransformerFunctions.hpp"
#include "src/core/Modules/Transformer/transformer.hpp"
#include "src/core/Modules/Transformer/TransformBuilder.hpp"
#include "src/core/Modules/MRGenerator/MRBuilder.hpp"

#include <map>

/*
Why wrap pow() in poww()?
cmath pow() contains many overloads. std::function<> template within TestableFunctionBase cannot deduce which
overload to use. It's easy to mitigate via slim wrappers like poww, for example.
*/
inline double poww(double x, double y) {
    return pow(x, y);
}

namespace MetamorphicRelationGenTests {
    inline void MetamorphicRelationTest_Pow() {
        constexpr size_t expectedMatchCount = 260;
        // MR: pow(x, y + 1) == pow(x, y) * x
#pragma region Data Preparation
        // Prepare an array of transformer functions
        const std::vector<std::function<void(double&, double)>> inputTransformerFuncs = { Div, Mul, Add, Sub, Noop};
        const std::vector<std::function<void(double&, double)>> outputTransformerFuncs = { Div, Mul, Add, Sub};

        // Prepare an array of matching argument arrays for each transformer. 1 to 1 matching
        std::vector<std::vector<double>> inputTransformerArgPool;
        std::vector<std::vector<double>> outputTransformerArgPool;

        inputTransformerArgPool.push_back({1.0, 2.0, -1.0});
        inputTransformerArgPool.push_back({1.0, 2.0, -1.0});
        inputTransformerArgPool.push_back({1.0, 2.0, -1.0});
        inputTransformerArgPool.push_back({1.0, 2.0, -1.0});
        inputTransformerArgPool.push_back({0.0});

        outputTransformerArgPool.push_back({1.0, 2.0, -1.0});
        outputTransformerArgPool.push_back({1.0, 2.0, -1.0});
        outputTransformerArgPool.push_back({1.0, 2.0, -1.0});
        outputTransformerArgPool.push_back({1.0, 2.0, -1.0});

        // Pow Function accepts args with type "double" only, hence we only need to prepare a pool of transformers for this data type.
        std::vector<std::shared_ptr<Aya::ITransformer>> doubleTransformers = Aya::TransformBuilder<double, double>().GetTransformers(inputTransformerFuncs, inputTransformerArgPool);
        std::vector<std::shared_ptr<Aya::ITransformer>> doubleTransformersForOutput = Aya::TransformBuilder<double, double>().GetTransformers(outputTransformerFuncs, outputTransformerArgPool);

        // Map transformers to matching indices of args (start from 0)
        // double Pow(double x, double y)
        std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
        inputTransformerPool.insert({0, doubleTransformers}); // x
        inputTransformerPool.insert({1, doubleTransformers}); // y

        std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> outputTransformerPool;
        outputTransformerPool.insert({0, doubleTransformersForOutput}); // return value
#pragma endregion

        size_t overallMatchCount = 0;
        // MR Builder for a function returning type double, tracked output type double, and two arguments of type double
        // double pow(double, double)
        auto mrBuilder = Aya::MRBuilder<double, double, double, double>(poww,
            inputTransformerPool, outputTransformerPool, outputTransformerFuncs, {0, 1}, 0);
        std::vector<std::vector<std::any>> testedInputs;
        std::vector<Aya::MetamorphicRelation> finalMRs;
        testedInputs.push_back({10.0, 11.0, 12.0});
        testedInputs.push_back({2.0, 3.0, 4.0});
        mrBuilder.SearchForMRs(testedInputs, 1, 1, overallMatchCount, finalMRs);

        TEST_EXPECT(overallMatchCount == expectedMatchCount);
        TEST_EXPECT(finalMRs.size() == expectedMatchCount);
    }
}