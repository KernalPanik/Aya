#pragma once
#include <iostream>

#include "LinalgUtils/LinalgTransformers.hpp"
#include "AyaCore.h"

inline std::array<double, 2> Vec2(std::array<double, 2> const &v) {
    return v;
}

inline void GenerateMRsForMatrixMul(const std::function<std::array<double, 2>(std::array<double, 2>)> &testedFunction,
                                    const std::function<bool(std::array<double, 2>, std::array<double, 2>)> &comparer,
                                    const std::string &outputMRFile,
                                    size_t inputTransformerChainLength,
                                    size_t outputTransformerChainLength,
                                    size_t leftValueIndex,
                                    size_t rightValueIndex,
                                    const std::vector<std::vector<std::any>> &testedInputs,
                                    const std::vector<std::vector<std::any>> &validatorInputs) {

    const std::vector<std::function<void(std::array<double, 2>&)>> singleArgumentTransformerFunctions = {Accelerate_Rotate15deg, Accelerate_Rotate30deg, Accelerate_Rotate60deg, Accelerate_Rotate90deg,
        Accelerate_Rotate120deg, Accelerate_Rotate150deg, Accelerate_RotateNeg15deg, Accelerate_RotateNeg30deg};
    const std::vector<std::string> singleArgumentTransformerFunctionNames = {"Rotate15Deg", "Rotate30Deg", "Rotate60Deg", "Rotate90Deg", "Rotate120Deg", "Rotate150Deg", "RotateNegative15Deg", "RotateNegative30Deg"};


    std::vector<std::shared_ptr<Aya::ITransformer>> singleArgumentTransformersForInputs = Aya::TransformBuilder<
        std::array<double,2>>().GetTransformers(singleArgumentTransformerFunctions, singleArgumentTransformerFunctionNames);
    std::vector<std::shared_ptr<Aya::ITransformer>> singleArgumentTransformersForOutputs = Aya::TransformBuilder<
            std::array<double,2>>().GetTransformers(singleArgumentTransformerFunctions, singleArgumentTransformerFunctionNames);

    std::vector<std::shared_ptr<Aya::ITransformer>> inputTransformers;
    inputTransformers.reserve(singleArgumentTransformersForInputs.size());
    inputTransformers.insert(inputTransformers.end(), singleArgumentTransformersForInputs.begin(),
                         singleArgumentTransformersForInputs.end());

    std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformers;
    outputTransformers.reserve(singleArgumentTransformersForInputs.size());
    outputTransformers.insert(outputTransformers.end(), singleArgumentTransformersForOutputs.begin(),
                                  singleArgumentTransformersForOutputs.end());
    std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
    inputTransformerPool.insert({0, inputTransformers});

    std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformerPool;
    outputTransformerPool.insert(outputTransformerPool.end(), outputTransformers.begin(), outputTransformers.end());

    auto mrBuilder = Aya::MRBuilder<std::array<double, 2>, std::array<double, 2>, std::array<double, 2>>(testedFunction, comparer, inputTransformerPool, outputTransformerPool,
                                                            leftValueIndex, rightValueIndex, outputTransformers, {});

    size_t overallMatchCount = 0;
    std::vector<Aya::MetamorphicRelation> finalMRs;
    mrBuilder.SearchForMRs(testedInputs, inputTransformerChainLength, outputTransformerChainLength, overallMatchCount, finalMRs);
    Aya::CalculateMRScore<std::array<double, 2>, std::array<double, 2>, std::array<double, 2>>(testedFunction, comparer, finalMRs,
                                                     validatorInputs, leftValueIndex, rightValueIndex);
    Aya::ProduceMREvaluationReport(finalMRs, validatorInputs, vec2str, outputMRFile);
}
