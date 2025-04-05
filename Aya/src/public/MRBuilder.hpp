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
        //TODO: rework MRBuilder:
        /// pass prebuilt variable output transformers. Allow ITransformer args to be overridden.
        /// pass matching vectors of indices that can be applied to an arg. if passed 3 items in a vector, an ITransformer with 3 args is assumed.
        /// This will improve MR search by allowing to combine various output variable transformers, even gluing them with constant transformers.
        // Transformer pools are generated with TransformBuilder class. MRBuilder extracts iterators from them.
        MRBuilder(std::function<T(Args...)> testedFunction, std::map<size_t, std::vector<std::shared_ptr<ITransformer>>>& inputTransformerPool,
                    const std::vector<std::shared_ptr<ITransformer>>& outputTransformerPool,
                    const size_t targetOutputTransformIndex,
                    const std::vector<std::shared_ptr<ITransformer>>& outputVariableTransformerPool,    // Output variable transformers to be overridden
                    const std::vector<std::vector<size_t>>& matchingOutputIndices)                      // Indices of arguments to use as an override. If vec is empty, assumed that no arg transform is executed.
                        : m_TestedFunction(testedFunction),
                        m_InputTransformerPool(inputTransformerPool),
                        m_OutputConstantTransformerPool(outputTransformerPool),
                        m_TargetOutputTransformIndex(targetOutputTransformIndex),
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

            m_OutputTransformerCounts.reserve(outputTransformerPool.size());
            m_OutputTransformerCounts.push_back(outputTransformerPool.size());
            /*for (auto &[transformers] : outputTransformerPool) {
                m_OutputTransformerCounts.push_back(transformers.size());
            }*/

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
            std::vector outputIteratorsTmp(outputTransformChainLength, CartesianIterator(m_OutputTransformerCounts));

            CompositeCartesianIterator inputIterator(inputIteratorsTmp);
            CompositeCartesianIterator outputIterator(outputIteratorsTmp);

            potentialMRCount = 0;
            std::vector<std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>> outputTransformerChains;
// TODO: move Output transform chain generation to MRContext exclusively?
#pragma region Output transform chain generation
            while (!outputIterator.isDone()) {
                std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformerChain;
                auto pos = outputIterator.getPos();
                for (auto &p : pos) {
                    for (auto &i : p) {
                        auto pair = std::make_shared<std::pair<size_t,
                            std::shared_ptr<ITransformer>>>(std::make_pair(m_TargetOutputTransformIndex, m_OutputConstantTransformerPool[i]));
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
                    auto ctx = Aya::MRContext<T, U, Args...>(m_TestedFunction, inputTransformerChain, m_OutputConstantTransformerPool, m_OutputVariableTransformerPool, m_TargetOutputTransformIndex, m_MatchingOutputVariableIndices, outputTransformChainLength);
                    ctx.SetImplicitOutputTransforms(m_EnableImplicitOutputTransforms);
                    while (!funcInputIterator.isDone()) {
                        auto funcInputPos = funcInputIterator.getPos();
                        std::vector<std::any> formedInputs;
                        formedInputs.reserve(funcInputPos.size());
                        for (size_t j = 0; j < funcInputPos.size(); j++) {
                            formedInputs.push_back(testedInputs[j][funcInputPos[j]]);
                        }
                        ctx.ValidateTransformChains(formedInputs, m_TargetOutputTransformIndex, mrs);
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
        const size_t m_TargetOutputTransformIndex;
        bool m_EnableImplicitOutputTransforms;
        std::vector<std::shared_ptr<ITransformer>> m_OutputVariableTransformerPool;
        std::vector<std::vector<size_t>> m_MatchingOutputVariableIndices;

        std::function<bool(U, U)> m_Comparer;

        std::vector<size_t> m_InputTransformerCounts;
        std::vector<size_t> m_OutputTransformerCounts;
    };
}