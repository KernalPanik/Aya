#pragma once
#include <cmath>
#include <vector>
#include <string>

#include "AyaCore.h"
#include "DoubleTypeTransformers.hpp"

// Driver for functions of signature double(double) "double double"

inline double SineSquared(const double x) {
    return sin(x * PI / 180) * sin(x * PI / 180);
}

inline double CosineSquared(const double x) {
    return cos(x * PI / 180) * cos(x * PI / 180);
}

inline double RootFunc(const double x) {
  	return sqrt(x);
}

inline double ExpFunc(const double x) {
  	return exp(x);
}

inline double LogFunc(const double x) {
  	return log(x);
}

inline double TanFunc(const double x) {
  	return tan(x * PI / 180);
}

inline double RadToDeg(const double x) {
    return x * 180 / PI;
}

inline double AsinFunc(const double x) {
    if (x > 1 || x < -1) {
        throw std::domain_error("Invalid argument");
    }
	return RadToDeg(asin(x));
}

inline double AcosFunc(const double x) {
    if (x > 1 || x < -1) {
        throw std::domain_error("Invalid argument");
    }
  	return RadToDeg(acos(x));
}

inline double AtanFunc(const double x) {
  	return RadToDeg(atan(x));
}

inline void GenerateMRsForDoubleDoubleArgFunc(const std::function<double(double)> &testedFunction,
        const std::function<bool(double, double)> &comparer,
        const std::string &outputMRFile,
        size_t inputTransformerChainLength,
        size_t outputTransformerChainLength,
        size_t leftValueIndex,
        size_t rightValueIndex,
        const std::vector<std::vector<std::any>> &testedInputs,
        const std::vector<std::vector<std::any>> &validatorInputs) {
    const std::vector<std::function<void(double &)>> singleArgumentTransformerFunctions = {Cos, Sin, CosDivSin, SinDivCos, Tan, Asin, Acos, Atan, Sin2, Cos2, Square};
    const std::vector<std::string> singleArgumentTransformerFunctionNames = {"Cos", "Sin", "CosDivBySin", "SinDivByCos", "Tan", "Asin", "Acos", "Atan", "SinSquared", "CosSquared", "Square"};
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

    std::vector<std::vector<size_t>> variableTransformerIndices = {{}, {}, {}, {}, {}, {}, {}, {}, {}, {}};
    for (size_t i = 3; i < inputTransformers.size() - 1; ++i) {
        variableTransformerIndices.push_back({0});
    }
    auto mrBuilder = Aya::MRBuilder<double, double, double>(testedFunction, comparer, inputTransformerPool, outputTransformerPool,
                                                            leftValueIndex, rightValueIndex, outputTransformers, variableTransformerIndices);
    mrBuilder.SetEnableImplicitOutputTransforms(true);

    size_t overallMatchCount = 0;
    std::vector<Aya::MetamorphicRelation> finalMRs;
    mrBuilder.SearchForMRs(testedInputs, inputTransformerChainLength, outputTransformerChainLength, overallMatchCount, finalMRs);

    Aya::CalculateMRScore<double, double, double>(static_cast<std::function<double(double)>>(testedFunction), comparer, finalMRs,
                                                  validatorInputs, leftValueIndex, rightValueIndex);
    Aya::ProduceMREvaluationReport(finalMRs, validatorInputs, doubleTypeToString, outputMRFile);
}
