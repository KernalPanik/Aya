#pragma once

#include "aya/Aya.hpp"
#include "TransformerFunctions.hpp"

inline std::vector<int> VecInit(std::vector<int> v) {
    return v;
}

inline std::string vec2str(const std::any &v) {
    std::stringstream ss;
    auto val = std::any_cast<std::vector<int>>(v);
    for (auto &i : val) {
        ss << i << ' ';
    }
    return ss.str();
}

inline void MetamorphicRelationTest_VectorSize(std::string outputFilename) {
    Aya::TransformerFunctionRegistry<std::vector<int>, int, int> pushRegistry;
    pushRegistry.RegisterFunction("PushNTimes", pushNTimes, {{1, 2}, {2, 1}, {1, 2}, {2, 2}});

    Aya::TransformerFunctionRegistry<std::vector<int>, int> popRegistry;
    popRegistry.RegisterFunction("PopNTimes", popNTimes, {{1}, {2}, {3}});

    // Combine all transformers for input and output
    std::vector<std::shared_ptr<Aya::ITransformer>> pushTransformers =
        Aya::TransformBuilder<std::vector<int>, int, int>().GetTransformers(
            pushRegistry.GetFuncs(), pushRegistry.GetNames(), pushRegistry.GetArgPools());

    std::vector<std::shared_ptr<Aya::ITransformer>> popTransformers =
        Aya::TransformBuilder<std::vector<int>, int>().GetTransformers(
            popRegistry.GetFuncs(), popRegistry.GetNames(), popRegistry.GetArgPools());

    std::vector<std::shared_ptr<Aya::ITransformer>> transformers;
    transformers.reserve(pushTransformers.size() + popTransformers.size());
    transformers.insert(transformers.end(), popTransformers.begin(), popTransformers.end());
    transformers.insert(transformers.end(), pushTransformers.begin(), pushTransformers.end());

    // Both input and output are vectors, so transformers match as well
    std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
    inputTransformerPool.insert({0, transformers});

    std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformerPool = transformers;

    auto mrBuilder = Aya::MRBuilder<std::vector<int>, std::vector<int>, std::vector<int>>(
        VecInit, nullptr, inputTransformerPool, outputTransformerPool, 0, 0, {}, {});
    mrBuilder.SetEnableImplicitOutputTransforms(false);

    std::vector<std::vector<std::any>> testedInputs;
    testedInputs.push_back({std::vector<int>({1, 2, 3}),
                            std::vector<int>(),
                            std::vector<int>({0})});

    size_t overallMatchCount = 0;
    std::vector<Aya::MetamorphicRelation> finalMRs;
    mrBuilder.SearchForMRs(testedInputs, 1, 2, overallMatchCount, finalMRs);

    std::cout << overallMatchCount << std::endl;
    std::cout << finalMRs.size() << std::endl;

    std::vector<std::vector<std::any>> validatorInputs;
    validatorInputs.push_back({std::vector<int>({1, 2, 3, 4}),
                                std::vector<int>(),
                                std::vector<int>{923231, 111142, 123},
                                std::vector<int>{1}});

    Aya::CalculateMRScore<std::vector<int>, std::vector<int>, std::vector<int>>(
        static_cast<std::function<std::vector<int>(std::vector<int>)>>(VecInit), nullptr, finalMRs, validatorInputs, 0, 0);
    Aya::ProduceMREvaluationReport(finalMRs, validatorInputs, inputTransformerPool.size(), outputTransformerPool.size(), 1, 1, vec2str, std::string(outputFilename));
}