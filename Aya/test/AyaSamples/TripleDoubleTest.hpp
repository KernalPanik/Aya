#pragma once

#include <cmath>

// Driver for functions of signature double(double, double) "triple double"

inline double Pow(double x, double y) {
    return pow(x, y);
}

inline void GenerateMRsForPow(const std::function<double(double, double)> &testedFunction,
   		const std::function<bool(double, double)> &comparer,
    	const std::string &outputMRFile,
    	size_t inputTransformerChainLength,
    	size_t outputTransformerChainLength,
        size_t leftValueIndex,
        size_t rightValueIndex,
    	std::vector<std::vector<std::any>> testedInputs,
    	std::vector<std::vector<std::any>> validationInputs) {
    // Prepare an array of transformer functions
    const std::vector<std::function<void(double &, double)>> inputTransformerFuncs = {Div, Mul, Add, Sub, Noop};
    const std::vector<std::string> inputTransformNames = {"Div", "Mul", "Add", "Sub", "Noop"};
    const std::vector<std::function<void(double &, double)>> outputTransformerFuncs = {Div, Mul, Add, Sub};
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
    std::vector<std::shared_ptr<Aya::ITransformer>> doubleTransformers = Aya::TransformBuilder<double, double>().
            GetTransformers(inputTransformerFuncs, inputTransformNames, inputTransformerArgPool);
    std::vector<std::shared_ptr<Aya::ITransformer>> doubleTransformersForOutput = Aya::TransformBuilder<double,
        double>().GetTransformers(outputTransformerFuncs, outputTransformNames, outputTransformerArgPool);

    // Map transformers to matching indices of args (start from 0)
    // double Pow(double x, double y)
    std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer> > > inputTransformerPool;
    inputTransformerPool.insert({0, doubleTransformers}); // x
    inputTransformerPool.insert({1, doubleTransformers}); // y

    size_t overallMatchCount = 0;
    // MR Builder for a function returning type double, tracked output type double, and two arguments of type double
    // double pow(double, double)

    std::vector<std::vector<size_t> > usableIndices;
    usableIndices.reserve(doubleTransformersForOutput.size());
    for (size_t i = 0; i < doubleTransformersForOutput.size(); i++) {
        std::vector<size_t> tmp = {0, 1};
        usableIndices.emplace_back(tmp);
    }

    auto mrBuilder = Aya::MRBuilder<double, double, double, double>(testedFunction, comparer,
                                                                    inputTransformerPool, doubleTransformersForOutput,
                                                                    leftValueIndex, rightValueIndex, doubleTransformersForOutput, usableIndices);
    mrBuilder.SetEnableImplicitOutputTransforms(true);
    std::vector<Aya::MetamorphicRelation> finalMRs;
    mrBuilder.SearchForMRs(testedInputs, inputTransformerChainLength, outputTransformerChainLength, overallMatchCount, finalMRs);

    Aya::CalculateMRScore<double, double, double, double>(
        static_cast<std::function<double(double, double)>>(testedFunction), comparer, finalMRs, validationInputs, leftValueIndex, rightValueIndex);
    Aya::ProduceMREvaluationReport(finalMRs, outputMRFile);
}