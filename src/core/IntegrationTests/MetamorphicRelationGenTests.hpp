#pragma once

#include "test/Framework/testRunnerUtils.h"
#include "src/core/IntegrationTests/TestUtils/TransformerFunctions.hpp"
#include "src/core/Modules/Transformer/transformer.hpp"
#include "src/core/Modules/Transformer/TransformBuilder.hpp"
#include "src/core/Modules/MRGenerator/MRBuilder.hpp"

#include <map>
#include <src/common/util.hpp>

/*
Why wrap pow() in poww()?
cmath pow() contains many overloads. std::function<> template within TestableFunctionBase cannot deduce which
overload to use. It's easy to mitigate via slim wrappers like poww, for example.
*/
inline double poww(double x, double y) {
    return pow(x, y);
}

inline size_t VecSize(std::vector<int> v) {
    return v.size();
}

namespace MetamorphicRelationGenTests {
    inline void MetamorphicRelationTest_Pow() {
        constexpr size_t expectedMatchCount = 260;
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

        std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> outputTransformerPool;
        outputTransformerPool.insert({0, doubleTransformersForOutput}); // return value

        size_t overallMatchCount = 0;
        // MR Builder for a function returning type double, tracked output type double, and two arguments of type double
        // double pow(double, double)
        auto mrBuilder = Aya::MRBuilder<double, double, double, double>(poww,
            inputTransformerPool, outputTransformerPool, outputTransformerFuncs, outputTransformNames, {0, 1}, 0);
        mrBuilder.SetEnableImplicitOutputTransforms(true);
        std::vector<std::vector<std::any>> testedInputs;
        std::vector<Aya::MetamorphicRelation> finalMRs;
        testedInputs.push_back({10.0, 11.0, 12.0});
        testedInputs.push_back({2.0, 3.0, 4.0});
        mrBuilder.SearchForMRs(testedInputs, 1, 1, overallMatchCount, finalMRs);


        for (auto &mr : finalMRs) {
            std::cout << mr.ToString() << std::endl;
        }

        TEST_EXPECT(overallMatchCount == expectedMatchCount);
        TEST_EXPECT(finalMRs.size() == expectedMatchCount);
    }

    inline void MetamorphicRelationTest_VectorSize() {
#pragma region Data Preparation
        const std::function inputPushTransformerFunc(push);
        const std::function inputPopTransformerFunc(pop);

        const std::vector valsToPush = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        const std::vector repeatVals = { 0, 1, 2, 3 };
        std::vector<std::tuple<int, int>> pushArgs;
        pushArgs.reserve(valsToPush.size() * repeatVals.size());
        for (auto &val : valsToPush) {
            for (auto &r : repeatVals) {
                pushArgs.emplace_back(val, r);
            }
        }

        std::vector<std::tuple<int>> popArgs;
        popArgs.reserve(repeatVals.size());
        for (auto& r : repeatVals) {
            popArgs.emplace_back(r);
        }

        //TODO: GetTransformers accept function names vector to match them to according function pointers.
        std::vector<std::shared_ptr<Aya::ITransformer>> pushTransformers = Aya::TransformBuilder<std::vector<int>, int, int>().GetTransformers(inputPushTransformerFunc, "Push", pushArgs);
        std::vector<std::shared_ptr<Aya::ITransformer>> popTransformers = Aya::TransformBuilder<std::vector<int>, int>().GetTransformers(inputPopTransformerFunc, "Pop", popArgs);

        std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
        inputTransformerPool.insert({0, pushTransformers});
        inputTransformerPool.insert({0, popTransformers});

        std::vector<std::vector<std::tuple<size_t>>> outputTransformArgPool;
        outputTransformArgPool.push_back({{0}, {1}, {2}, {3}});
        outputTransformArgPool.push_back({{0}, {1}, {2}, {3}});
        const std::vector<std::function<void(size_t&, size_t)>> outputTransformerFuncs = {VecAdd, VecSub };
        const std::vector<std::string> outputTransformNames = {"VecSizeAdd", "VecSizeSub"};

        std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformers = Aya::TransformBuilder<size_t, size_t>().GetTransformers(outputTransformerFuncs, outputTransformNames, outputTransformArgPool);
        std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> outputTransformerPool;
        outputTransformerPool.insert({0, outputTransformers});
#pragma endregion

        size_t overallMatchCount = 0;
        auto mrBuilder = Aya::MRBuilder<size_t, size_t, std::vector<int>>(VecSize,
            inputTransformerPool, outputTransformerPool, outputTransformerFuncs, outputTransformNames, {0}, 0);
        mrBuilder.SetEnableImplicitOutputTransforms(false);
        std::vector<std::vector<std::any>> testedInputs;
        std::vector<Aya::MetamorphicRelation> finalMRs;

        testedInputs.emplace_back(std::vector<std::any>{std::vector<int>{1}});
        testedInputs.emplace_back(std::vector<std::any>{std::vector<int>{5}});
        mrBuilder.SearchForMRs(testedInputs, 1, 1, overallMatchCount, finalMRs);
        TEST_EXPECT(overallMatchCount == 10);
    }
}