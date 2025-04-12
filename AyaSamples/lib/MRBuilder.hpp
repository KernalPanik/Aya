#pragma once
#include "transformer.hpp"
#include "CompositeCartesianIterator.h"
#include "MRContext.hpp"

#include <vector>
#include <any>
#include <map>

namespace Aya {
    // T -- tested function return type
    // U -- tracked output type
    // Args -- tested function args
    template <typename T, typename U, typename... Args>
    class MRBuilder {
    public:
        MRBuilder(std::function<T(Args...)> testedFunction, std::map<size_t, std::vector<std::shared_ptr<ITransformer>>>& inputTransformerPool,
                    const std::vector<std::shared_ptr<ITransformer>>& outputTransformerPool,
                    const size_t targetLeftValueIndex,
                    const size_t targetRightValueIndex,
                    const std::vector<std::shared_ptr<ITransformer>>& outputVariableTransformerPool,    // Output variable transformers to be overridden
                    const std::vector<std::vector<size_t>>& matchingOutputIndices)                      // Indices of arguments to use as an override. If vec is empty, assumed that no arg transform is executed.
                        : m_TestedFunction(testedFunction),
                        m_InputTransformerPool(inputTransformerPool),
                        m_OutputConstantTransformerPool(outputTransformerPool),
                        m_TargetLeftValueIndex(targetLeftValueIndex),
                        m_TargetRightValueIndex(targetRightValueIndex),
                        m_OutputVariableTransformerPool(outputVariableTransformerPool), // indices of input state that can be passed to the output transform. To be replaced by matchingOutputIndices
                        m_MatchingOutputVariableIndices(matchingOutputIndices) {
            if (m_InputTransformerPool.empty()) {
                throw std::invalid_argument("Input transform pool is not initialized");
            }

            if (m_OutputConstantTransformerPool.empty()) {
                throw std::invalid_argument("Output constant transform pool is not initialized.");
            }

            m_InputTransformerCounts.reserve(inputTransformerPool.size());
            for (auto &[index, transformers] : inputTransformerPool) {
                m_InputTransformerCounts.push_back(transformers.size());
            }

            m_EnableImplicitOutputTransforms = false;
        }
        ~MRBuilder() = default;

        void SetComparer(std::function<void(U, U)> comparer) {
            m_Comparer = comparer;
        }

        void SetEnableImplicitOutputTransforms(const bool value) {
            m_EnableImplicitOutputTransforms = value;
        }

        void SearchForMRs(std::vector<std::vector<std::any>>& testedInputs, const size_t inputTransformChainLength,
                const size_t outputTransformChainLength, size_t& potentialMRCount, std::vector<MetamorphicRelation>& metamorphicRelations) {
            std::vector inputIteratorsTmp(inputTransformChainLength, CartesianIterator(m_InputTransformerCounts));

            CompositeCartesianIterator inputIterator(inputIteratorsTmp);

            potentialMRCount = 0;
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> inputTransformerChain;
            std::vector<size_t> functionInputLengths;
            functionInputLengths.reserve(testedInputs.size());
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
                std::vector<MetamorphicRelation> mrs;

                //for (auto &otc : outputTransformerChains) {
                    auto ctx = Aya::MRContext<T, U, Args...>(m_TestedFunction, inputTransformerChain, m_OutputConstantTransformerPool, m_OutputVariableTransformerPool, m_TargetLeftValueIndex, m_TargetRightValueIndex, m_MatchingOutputVariableIndices, outputTransformChainLength);
                    ctx.SetImplicitOutputTransforms(m_EnableImplicitOutputTransforms);
                    while (!funcInputIterator.isDone()) {
                        auto funcInputPos = funcInputIterator.getPos();
                        std::vector<std::any> formedInputs;
                        formedInputs.reserve(funcInputPos.size());
                        for (size_t j = 0; j < funcInputPos.size(); j++) {
                            formedInputs.push_back(testedInputs[j][funcInputPos[j]]);
                        }
                        ctx.ValidateTransformChains(formedInputs, m_TargetLeftValueIndex, m_TargetRightValueIndex, mrs);
                        funcInputIterator.next();
                        formedInputs.clear();
                    }
                    if (ctx.GetTotalMatches() != 0) {
                        potentialMRCount += ctx.GetTotalMatches();
                        metamorphicRelations.insert(metamorphicRelations.end(), mrs.begin(), mrs.end());
                    }
                    mrs.clear();
                //}
                inputIterator.next();
                inputTransformerChain.clear();
            }
        }

    private:
        std::function<T(Args...)> m_TestedFunction;
        std::map<size_t, std::vector<std::shared_ptr<ITransformer>>>& m_InputTransformerPool;
        std::vector<std::shared_ptr<ITransformer>> m_OutputConstantTransformerPool;
        const size_t m_TargetLeftValueIndex;
        const size_t m_TargetRightValueIndex;
        bool m_EnableImplicitOutputTransforms;
        std::vector<std::shared_ptr<ITransformer>> m_OutputVariableTransformerPool;
        std::vector<std::vector<size_t>> m_MatchingOutputVariableIndices;

        std::function<bool(U, U)> m_Comparer;

        std::vector<size_t> m_InputTransformerCounts;
    };
}