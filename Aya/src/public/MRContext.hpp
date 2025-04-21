#pragma once

#include "CoreUtilities.hpp"
#include "transformer.hpp"
#include "MetamorphicRelation.hpp"

#include <functional>
#include <iostream>
#include <string>
#include <any>
#include <algorithm>
#include <map>

namespace Aya {
    class IMRContext {
    public:
        virtual ~IMRContext() = default;

        // std::vector<std::any> is an abstract class friendly way of having a tuple. So MRSearch would accept a vector of vectors, encompassing inputs.
        virtual void ValidateTransformChains(const std::vector<std::any> &inputs, size_t leftValueIndex,
                                             size_t rightValueIndex,
                                             std::vector<Aya::MetamorphicRelation> &metamorphicRelations) = 0;

        [[nodiscard]]
        virtual size_t GetTotalMatches() const = 0;

        virtual void SetImplicitOutputTransforms(bool value) = 0;
    };

    template<typename T, typename U, typename... Args>
    class MRContext final : public IMRContext {
    public:
        using ReturnType = std::conditional_t<
            std::is_void_v<T>,
            std::unique_ptr<int>,
            T>;

        explicit MRContext(std::function<T(Args...)> f,
                           std::function<bool(U, U)> comparer,
                           const std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>
                           &inputTransformChain,
                           const std::vector<std::shared_ptr<ITransformer>> &outputConstantTransformerPool,
                           const std::vector<std::shared_ptr<ITransformer>> &outputVariableTransformerPool,
                           // Indices of arguments to use as an override. If vec is empty, assumed that no arg transform is executed.
                           const std::vector<std::vector<size_t>> &matchingOutputIndices,
                           const size_t outputTransformChainLength)
            : m_Func(std::move(f)),
              m_InputTransforms(inputTransformChain),
              m_MatchingArgumentIndices(matchingOutputIndices),
              m_Comparer(comparer),
              m_OutputConstantTransformers(outputConstantTransformerPool),
              m_OutputVariableTransformers(outputVariableTransformerPool),
              m_OutputTransformChainLength(outputTransformChainLength) {
            std::sort(m_InputTransforms.begin(), m_InputTransforms.end(), [](auto &left, auto &right) {
                return left->first < right->first;
            });

            m_TotalMatches = 0;
            m_BuildImplicitOutputTransforms = false;
        }

        [[nodiscard]]
        size_t GetTotalMatches() const override {
            return m_TotalMatches;
        }

        void SetImplicitOutputTransforms(const bool value) override {
            m_BuildImplicitOutputTransforms = value;
        }

        void ValidateTransformChains(const std::vector<std::any> &inputs, const size_t leftValueIndex,
                                     const size_t rightValueIndex,
                                     std::vector<MetamorphicRelation> &metamorphicRelations) override {
            auto initialStateVector = InvokeInternal(inputs);
            std::vector<std::any> followUpInputs;
            try {
                followUpInputs = TransformInputs(inputs, std::index_sequence_for<Args...>{});
            }
            catch (std::domain_error &e) {
                return;
            }
            auto followUpStateVec = InvokeInternal(followUpInputs);

            std::vector<std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>>
                    generatedOutputTransformChains;
            std::vector<std::shared_ptr<ITransformer>> totalOutputTransformerPool;
            totalOutputTransformerPool.reserve(m_OutputConstantTransformers.size());
            totalOutputTransformerPool.insert(totalOutputTransformerPool.end(), m_OutputConstantTransformers.begin(),
                                              m_OutputConstantTransformers.end());

            // Samples are the transformed outputs trying to match changes from initial to follow up
            if (m_BuildImplicitOutputTransforms) {
                auto variableOutputTransformers = ProduceVariableOutputTransformers(
                    initialStateVector, rightValueIndex);
                totalOutputTransformerPool.insert(totalOutputTransformerPool.end(), variableOutputTransformers.begin(),
                                                  variableOutputTransformers.end());
            }

            const std::vector outputTransformIterators(m_OutputTransformChainLength,
                                                 CartesianIterator({totalOutputTransformerPool.size()}));
            CompositeCartesianIterator outputTransformIterator(outputTransformIterators);
            while (!outputTransformIterator.isDone()) {
                std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformChain;
                auto pos = outputTransformIterator.getPos();
                for (auto &p: pos) {
                    for (const auto &i: p) {
                        auto pair = std::make_shared<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>(
                            rightValueIndex, totalOutputTransformerPool[i]);
                        outputTransformChain.push_back(pair);
                    }
                }
                generatedOutputTransformChains.push_back(outputTransformChain);
                outputTransformIterator.next();
            }

            for (auto &outputTransformChain: generatedOutputTransformChains) {
                try {
                    if (auto sampleOutput = TransformOutputs(initialStateVector, outputTransformChain);
                        CompareTargetElements(std::any_cast<U>(followUpStateVec[leftValueIndex]),
                        std::any_cast<U>(sampleOutput[rightValueIndex]))) {
                        metamorphicRelations.emplace_back(m_InputTransforms, outputTransformChain, leftValueIndex, rightValueIndex);
                        m_TotalMatches++;
                    }
                }
                catch (std::domain_error &e) {}
            }
        }

    private:
        std::function<T(Args...)> m_Func;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>> m_InputTransforms;
        std::vector<std::vector<size_t>> m_MatchingArgumentIndices;
        // double(double, int) => index 0 can be used to transform output 'double'. Generally specified by the user of the API
        std::function<bool(U, U)> m_Comparer;
        std::vector<std::shared_ptr<ITransformer>> m_OutputConstantTransformers;
        std::vector<std::shared_ptr<ITransformer>> m_OutputVariableTransformers;
        const size_t m_OutputTransformChainLength;
        size_t m_TotalMatches = 0;
        bool m_BuildImplicitOutputTransforms = false;

        std::vector<std::any> InvokeInternal(const std::vector<std::any> &inputs) {
            std::vector<std::any> inputState = inputs;
            auto producedState = CaptureProducedState<T, U, Args...>(m_Func, inputState);

            return producedState;
        }

       /*
        * NOTE:
        * Effectively, this function always overrides the 'first' argument that comes after the base value.
        * This is a bug that was discovered recently. Transformers with multiple arguments, that are not possible
        * to be split into multiple smaller (one base, one modifier) transformers cannot be overridden.
        * Workaround: split larger transformers into several smaller ones.
        */
        std::vector<std::shared_ptr<ITransformer>> ProduceVariableOutputTransformers(
            std::vector<std::any> &stateVector, size_t targetOutputIndex) const {
            std::vector<std::shared_ptr<ITransformer>> newOutputTransformers;

            for (size_t i = 0; i < m_OutputVariableTransformers.size(); i++) {
                for (auto &index: m_MatchingArgumentIndices[i]) {
                    auto copp = m_OutputVariableTransformers[i]->Clone();
                    newOutputTransformers.push_back(copp);
                    newOutputTransformers.back()->OverrideArgs({stateVector[index + 1]}, index+1);
                    newOutputTransformers.back()->OverrideArgNames({"input[" + std::to_string(index) + "]"});
                }
            }
            return newOutputTransformers;
        }

        template<std::size_t... I>
        std::vector<std::any> TransformInputs(const std::vector<std::any> &inputs, std::index_sequence<I...>) {
            auto inputsCopy(inputs);

            for (const auto &t: m_InputTransforms) {
                t->second->Apply(inputsCopy[t->first]);
            }
            return inputsCopy;
        }

        std::vector<std::any> TransformOutputs(const std::vector<std::any> &outputs,
                                               const std::vector<std::shared_ptr<std::pair<size_t,
                                               std::shared_ptr<ITransformer>>>> &transformers) {
            auto outputsCopy(outputs);

            for (const auto &t: transformers) {
                t->second->Apply(outputsCopy[t->first]);
            }
            return outputsCopy;
        }

        bool CompareTargetElements(const U e1, const U e2) {
            if (m_Comparer) {
                return m_Comparer(e1, e2);
            }

            return e1 == e2;
        }
    };
}
