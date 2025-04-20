#pragma once
#include <cmath>
#include <vector>
#include <string>

#include "AyaCore.h"
#include "DoubleTypeTransformers.hpp"

inline double sineSquared(const double x) {
    return sin(x * PI / 180) * sin(x * PI / 180);
}

inline double cosineSquared(const double x) {
    return cos(x * PI / 180) * cos(x * PI / 180);
}

inline void GenerateMRsForSineOrCosLowPrecision(std::function<double(double)> testedFunction,
        const std::function<bool(double, double)> &comparer,
        const std::string &outputMRFile,
        size_t inputTransformerChainLength,
        size_t outputTransformerChainLength,
        const std::vector<std::vector<std::any>> &testedInputs,
        const std::vector<std::vector<std::any>> &validatorInputs) {
    const std::vector<std::function<void(double &)>> singleArgumentTransformerFunctions = {Cos, Sin};
    const std::vector<std::string> singleArgumentTransformerFunctionNames = {"Cos", "Sin"};
    const std::vector<std::function<void(double &, double)>> doubleArgumentTransformerFunctions = {Add, Mul, Sub, Div};
    const std::vector<std::string> doubleArgumentTransformerFunctionNames = {"Add", "Mul", "Sub", "Div"};

    // Vector of vectors (match every transformer function).
    std::vector<std::vector<std::tuple<double>>> inputTransformerArgumentPool;
    std::vector<std::vector<std::tuple<double>>> outputTransformerArgumentPool;

    inputTransformerArgumentPool.push_back({{-1}, {1.0}});
    inputTransformerArgumentPool.push_back({{-1.0}, {1.0}});
    inputTransformerArgumentPool.push_back({{1.0}, {-1.0}});
    inputTransformerArgumentPool.push_back({{1.0}, {-1.0}});

    outputTransformerArgumentPool.push_back({{-1}});
    outputTransformerArgumentPool.push_back({{-1.0}});
    outputTransformerArgumentPool.push_back({{1}});
    outputTransformerArgumentPool.push_back({{1.0}});

    std::vector<std::shared_ptr<Aya::ITransformer>> singleArgumentTransformersForInputs = Aya::TransformBuilder<
        double>().GetTransformers(singleArgumentTransformerFunctions, singleArgumentTransformerFunctionNames);
    std::vector<std::shared_ptr<Aya::ITransformer>> doubleArgumentTransformersForInputs = Aya::TransformBuilder<double,
        double>().GetTransformers(doubleArgumentTransformerFunctions, doubleArgumentTransformerFunctionNames,
                                  inputTransformerArgumentPool);
    std::vector<std::shared_ptr<Aya::ITransformer>> inputTransformers;
    inputTransformers.reserve(singleArgumentTransformersForInputs.size() + doubleArgumentTransformersForInputs.size());
    inputTransformers.insert(inputTransformers.end(), singleArgumentTransformersForInputs.begin(),
                             singleArgumentTransformersForInputs.end());
    inputTransformers.insert(inputTransformers.end(), doubleArgumentTransformersForInputs.begin(),
                             doubleArgumentTransformersForInputs.end());

    std::vector<std::shared_ptr<Aya::ITransformer>> singleArgumentTransformersForOutputs = Aya::TransformBuilder<
        double>().GetTransformers(singleArgumentTransformerFunctions, singleArgumentTransformerFunctionNames);
    std::vector<std::shared_ptr<Aya::ITransformer>> doubleArgumentTransformersForOutputs = Aya::TransformBuilder<double
        , double>().GetTransformers(doubleArgumentTransformerFunctions, doubleArgumentTransformerFunctionNames,
                                    outputTransformerArgumentPool);
    std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformers;
    outputTransformers.reserve(singleArgumentTransformersForInputs.size() + doubleArgumentTransformersForInputs.size());
    outputTransformers.insert(outputTransformers.end(), singleArgumentTransformersForOutputs.begin(),
                              singleArgumentTransformersForOutputs.end());
    outputTransformers.insert(outputTransformers.end(), doubleArgumentTransformersForOutputs.begin(),
                              doubleArgumentTransformersForOutputs.end());

    std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
    inputTransformerPool.insert({0, inputTransformers});

    std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformerPool;
    outputTransformerPool.insert(outputTransformerPool.end(), outputTransformers.begin(), outputTransformers.end());

    std::vector<std::vector<size_t>> variableTransformerIndices = {{}, {}, {}, {}};
    for (size_t i = 3; i < inputTransformers.size() - 1; ++i) {
        variableTransformerIndices.push_back({0});
    }
    auto mrBuilder = Aya::MRBuilder<double, double, double>(testedFunction, comparer, inputTransformerPool, outputTransformerPool,
                                                            0, 1, outputTransformers, variableTransformerIndices);
    mrBuilder.SetEnableImplicitOutputTransforms(true);

    size_t overallMatchCount = 0;
    std::vector<Aya::MetamorphicRelation> finalMRs;
    mrBuilder.SearchForMRs(testedInputs, inputTransformerChainLength, outputTransformerChainLength, overallMatchCount, finalMRs);

    Aya::CalculateMRScore<double, double, double>(static_cast<std::function<double(double)>>(testedFunction), equals, finalMRs,
                                                  validatorInputs, 0, 1);
    Aya::ProduceMREvaluationReport(finalMRs, outputMRFile);
}
