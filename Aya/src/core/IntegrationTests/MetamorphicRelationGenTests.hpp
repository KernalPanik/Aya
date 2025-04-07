#pragma once

#include "test/Framework/testRunnerUtils.h"
#include "src/core/IntegrationTests/TestUtils/TransformerFunctions.hpp"
#include "src/public/transformer.hpp"
#include "src/public/TransformBuilder.hpp"
#include "src/public/MRBuilder.hpp"

#include <map>

/*
Why wrap pow() in poww()?
cmath pow() contains many overloads. std::function<> template within TestableFunctionBase cannot deduce which
overload to use. It's easy to mitigate via slim wrappers like poww, for example.
*/
inline double poww(double x, double y) {
    return pow(x, y);
}

inline std::vector<int> VecInit(std::vector<int> v) {
    return v;
}

namespace MetamorphicRelationGenTests {
    // Implicit transform generation enabled
    // No custom comparer
    inline void MetamorphicRelationTest_Pow() {
        constexpr size_t expectedMatchCount = 699;
        // MR: pow(x, y + 1) == pow(x, y) * x
#pragma region Data Preparation
        // Prepare an array of transformer functions
        const std::vector<std::function<void(double&, double)>> inputTransformerFuncs = { Div, Mul, Add, Sub, Noop};
        const std::vector<std::string> inputTransformNames = {"Div", "Mul", "Add", "Sub", "Noop"};
        const std::vector<std::function<void(double&, double)>> outputTransformerFuncs = { Div, Mul, Add, Sub};
        const std::vector<std::string> outputTransformNames = {"Div", "Mul", "Add", "Sub"};

        // Prepare an array of matching argument arrays for each transformer. 1 to 1 matching
        std::vector<std::vector<std::tuple<double>>> inputTransformerArgPool;
        std::vector<std::vector<std::tuple<double>>> outputTransformerArgPool;

        inputTransformerArgPool.push_back({{1.0}, {2.0}, {-1.0}});
        inputTransformerArgPool.push_back({{1.0}, {2.0}, {-1.0}});
        inputTransformerArgPool.push_back({{1.0}, {2.0}, {-1.0}});
        inputTransformerArgPool.push_back({{1.0}, {2.0}, {-1.0}});
        inputTransformerArgPool.push_back({{0.0}});

        outputTransformerArgPool.push_back({{1.0}, {2.0}, {-1.0}});
        outputTransformerArgPool.push_back({{1.0}, {2.0}, {-1.0}});
        outputTransformerArgPool.push_back({{1.0}, {2.0}, {-1.0}});
        outputTransformerArgPool.push_back({{1.0}, {2.0}, {-1.0}});

        // Pow Function accepts args with type "double" only, hence we only need to prepare a pool of transformers for this data type.
        std::vector<std::shared_ptr<Aya::ITransformer>> doubleTransformers = Aya::TransformBuilder<double, double>().GetTransformers(inputTransformerFuncs, inputTransformNames, inputTransformerArgPool);
        std::vector<std::shared_ptr<Aya::ITransformer>> doubleTransformersForOutput = Aya::TransformBuilder<double, double>().GetTransformers(outputTransformerFuncs, outputTransformNames, outputTransformerArgPool);

        // Map transformers to matching indices of args (start from 0)
        // double Pow(double x, double y)
        std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
        inputTransformerPool.insert({0, doubleTransformers}); // x
        inputTransformerPool.insert({1, doubleTransformers}); // y

        std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformerPool;
        outputTransformerPool.insert(outputTransformerPool.end(), doubleTransformersForOutput.begin(), doubleTransformersForOutput.end()); // return value
#pragma endregion
        size_t overallMatchCount = 0;
        // MR Builder for a function returning type double, tracked output type double, and two arguments of type double
        // double pow(double, double)

        std::vector<std::vector<size_t>> usableIndices;
        usableIndices.reserve(doubleTransformersForOutput.size());
        for (size_t i = 0; i < doubleTransformersForOutput.size(); i++) {
            std::vector<size_t> tmp = {0, 1};
            usableIndices.emplace_back(tmp);
        }

        auto mrBuilder = Aya::MRBuilder<double, double, double, double>(poww,
            inputTransformerPool, outputTransformerPool, 0, doubleTransformersForOutput, usableIndices);
        mrBuilder.SetEnableImplicitOutputTransforms(true);
        std::vector<std::vector<std::any>> testedInputs;
        std::vector<Aya::MetamorphicRelation> finalMRs;
        testedInputs.push_back({10.0, 11.0, 12.0});
        testedInputs.push_back({2.0, 3.0, 4.0});
        mrBuilder.SearchForMRs(testedInputs, 1, 1, overallMatchCount, finalMRs);

        std::cout << overallMatchCount << std::endl;
        std::cout << finalMRs.size() << std::endl;

        TEST_EXPECT(overallMatchCount == expectedMatchCount);
        TEST_EXPECT(finalMRs.size() == expectedMatchCount);
    }

    // No implicit transform generation
    // Passing custom comparer
    inline void MetamorphicRelationTest_VectorSize() {
        const std::function pushTransform = push;
        const std::function popTransform = pop;
        const std::vector<std::string> transformNames = {"Push", "Pop" };

        std::vector<std::tuple<int, int>> pushTransformerArgPool;
        std::vector<std::tuple<int>> popTransformerArgPool;

        pushTransformerArgPool.emplace_back(1, 2);
        pushTransformerArgPool.emplace_back(2, 1);
        pushTransformerArgPool.emplace_back(1, 2);
        pushTransformerArgPool.emplace_back(2, 2);
        popTransformerArgPool.emplace_back(1);
        popTransformerArgPool.emplace_back(2);
        popTransformerArgPool.emplace_back(3);

        std::vector<std::shared_ptr<Aya::ITransformer>> pushTransformers = Aya::TransformBuilder<std::vector<int>, int, int>().GetTransformers(pushTransform, transformNames[0], pushTransformerArgPool);
        std::vector<std::shared_ptr<Aya::ITransformer>> popTransformers = Aya::TransformBuilder<std::vector<int>, int>().GetTransformers(popTransform, transformNames[1], popTransformerArgPool);
        std::vector<std::shared_ptr<Aya::ITransformer>> transformers;
        transformers.reserve(popTransformerArgPool.size() + pushTransformerArgPool.size());
        transformers.insert(transformers.end(), popTransformers.begin(), popTransformers.end());
        transformers.insert(transformers.end(), pushTransformers.begin(), pushTransformers.end());

        // Both input and output are vectors, so transformers match as well
        std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
        inputTransformerPool.insert({0, transformers});

        std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformerPool;
        outputTransformerPool.insert(outputTransformerPool.end(), transformers.begin(), transformers.end());

        auto mrBuilder = Aya::MRBuilder<std::vector<int>, std::vector<int>, std::vector<int>>(
            VecInit, inputTransformerPool, outputTransformerPool, 0, {}, {});
        mrBuilder.SetEnableImplicitOutputTransforms(false);
        std::vector<std::vector<std::any>> testedInputs;
        testedInputs.emplace_back();
        testedInputs[0].push_back(std::make_any<std::vector<int>>({1, 2, 3}));
        testedInputs[0].push_back(std::make_any<std::vector<int>>());
        testedInputs[0].push_back(std::make_any<std::vector<int>>({0}));

        size_t overallMatchCount = 0;
        std::vector<Aya::MetamorphicRelation> finalMRs;
        mrBuilder.SearchForMRs(testedInputs, 1, 2, overallMatchCount, finalMRs);
        TEST_EXPECT(overallMatchCount == 135);
    }
}