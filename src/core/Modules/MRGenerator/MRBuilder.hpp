// This header is expected to be the main entry point for Aya MR builder. Users typically would include it to generate MRs,
// And TransformBuilder.hpp to generate Transforms to be passed to MR Builder
#pragma once
#include "src/core/Modules/Transformer/transformer.h"
#include "src/Common/CompositeCartesianIterator.h"

#include <vector>
#include <any>
#include <map>

// T -- tested function return type
// Args -- tested function args
// U -- tracked output type
template <typename T, typename U, typename... Args>
class MRBuilder {
public:
    // Transformer pools are generated with TransformBuilder class. MRBuilder extracts iterators from them.
    MRBuilder(std::function<T(Args...)> testedFunction, std::map<size_t, std::vector<std::shared_ptr<ITransformer>>>& inputTransformerPool,
                std::map<size_t, std::vector<std::shared_ptr<ITransformer>>>& outputTransformerPool,
                std::vector<std::function<void(T&, T)>> outputTransformFunctions,
                const std::vector<size_t>& outputTransformerIndices,
                const size_t targetOutputTransformIndex)
                : m_TestedFunction(testedFunction),
                m_InputTransformerPool(inputTransformerPool),
                m_OutputTransformerPool(outputTransformerPool),
                m_OutputTransformFunctions(outputTransformFunctions),
                m_OutputTransformerIndices(outputTransformerIndices),
                m_TargetOutputTransformIndex(targetOutputTransformIndex) {
        m_InputTransformerCounts.reserve(inputTransformerPool.size());
        for (auto &[index, transformers] : inputTransformerPool) {
            m_InputTransformerCounts.push_back(transformers.size());
        }

        m_OutputTransformerCounts.reserve(outputTransformerPool.size());
        for (auto &[index, transformers] : outputTransformerPool) {
            m_OutputTransformerCounts.push_back(transformers.size());
        }
    }
    ~MRBuilder() = default;

    void SearchForMRs(std::vector<std::vector<std::any>>& testedInputs, const size_t inputTransformLength, const size_t outputTransformLength, size_t& potentialMRCount) {
        std::vector inputIteratorsTmp(inputTransformLength, CartesianIterator(m_InputTransformerCounts));
        std::vector inputIteratorsTmp1(inputTransformLength, CartesianIterator(m_InputTransformerCounts));

        for (auto &itr: inputIteratorsTmp1) {
            while (!itr.isDone()){ itr.next();}
        }

        std::vector outputIteratorsTmp(outputTransformLength, CartesianIterator(m_OutputTransformerCounts));

        CompositeCartesianIterator inputIterator(inputIteratorsTmp);
        CompositeCartesianIterator outputIterator(outputIteratorsTmp);

        potentialMRCount = 0;

        std::vector<std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>> outputTransformerChains;
#pragma region Output transform chain generation
        while (!outputIterator.isDone()) {
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformerChain;
            auto pos = outputIterator.getPos();
            for (auto &p : pos) {
                for (auto &i : p) {
                    auto pair = std::make_shared<std::pair<size_t,
                        std::shared_ptr<ITransformer>>>(std::make_pair(m_TargetOutputTransformIndex, m_OutputTransformerPool[m_TargetOutputTransformIndex][i]));
                    outputTransformerChain.push_back(pair);
                }
            }
            outputTransformerChains.push_back(outputTransformerChain);
            outputTransformerChain.clear();
            outputIterator.next();
        }
#pragma endregion
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> inputTransformerChain;
        std::vector<size_t> functionInputLengths;
        for (auto &i : testedInputs) {
            functionInputLengths.push_back(i.size());
        }
        while (!inputIterator.isDone()) {
            auto pos = inputIterator.getPos();
            size_t index = 0;
            for (auto &p : pos) {
                for (auto &i : p) {
                    auto pair = std::make_shared<std::pair<size_t, std::shared_ptr<ITransformer>>>(std::make_pair(index, m_InputTransformerPool[index][i]));
                    inputTransformerChain.push_back(pair);
                    index++;
                }
                index = 0;
            }
            auto funcInputIterator = CartesianIterator(functionInputLengths);
            for (auto &otc : outputTransformerChains) {
                auto ctx = TestContext<T, Args...>(m_TestedFunction, inputTransformerChain, otc, m_OutputTransformFunctions, m_OutputTransformerIndices);
                while (!funcInputIterator.isDone()) {
                    auto funcInputPos = funcInputIterator.getPos();
                    std::vector<std::any> formedInputs;
                    for (size_t j = 0; j < funcInputPos.size(); j++) {
                        formedInputs.push_back(testedInputs[j][funcInputPos[j]]);
                    }
                    ctx.ValidateTransformChains(formedInputs, m_TargetOutputTransformIndex);
                    funcInputIterator.next();
                    formedInputs.clear();
                }
                if (ctx.GetTotalMatches() != 0) {
                    potentialMRCount += ctx.GetTotalMatches();
                }
            }
            inputIterator.next();
            inputTransformerChain.clear();
        }
    }

private:
    std::function<T(Args...)> m_TestedFunction;
    std::map<size_t, std::vector<std::shared_ptr<ITransformer>>>& m_InputTransformerPool;
    std::map<size_t, std::vector<std::shared_ptr<ITransformer>>>& m_OutputTransformerPool;
    std::vector<std::function<void(U&, U)>> m_OutputTransformFunctions;
    std::vector<size_t> m_OutputTransformerIndices;
    const size_t m_TargetOutputTransformIndex;

    std::vector<size_t> m_InputTransformerCounts;
    std::vector<size_t> m_OutputTransformerCounts;
};
