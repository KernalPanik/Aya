#pragma once

#include "tuple-utils.h"
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
        virtual bool ValidateTransformChains(const std::vector<std::any>& inputs, size_t leftValueIndex, size_t rightValueIndex, std::vector<Aya::MetamorphicRelation>& metamorphicRelations) = 0;
        [[nodiscard]]
        virtual size_t GetTotalMatches() const = 0;
        virtual void SetImplicitOutputTransforms(bool value) = 0;
    };

    template <typename T, typename U, typename... Args>
    class MRContext final : public IMRContext {
    public:
        using ReturnType = std::conditional_t<
            std::is_void_v<T>,
            std::unique_ptr<int>,
            T>;

        explicit MRContext(std::function<T(Args...)> f,
            std::function<bool(U, U)> comparer,
            const std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>>& inputTransformChain,
            const std::vector<std::shared_ptr<ITransformer>>& outputConstantTransformerPool,
            const std::vector<std::shared_ptr<ITransformer>>& outputVariableTransformerPool,   // Output variable transformers to be overridden
            const size_t leftValueIndex,               // Indices of arguments to use as an override. If vec is empty, assumed that no arg transform is executed.
            const size_t rightValueIndex,               // Indices of arguments to use as an override. If vec is empty, assumed that no arg transform is executed.
            const std::vector<std::vector<size_t>>& matchingOutputIndices,
            const size_t outputTransformChainLength)               // Indices of arguments to use as an override. If vec is empty, assumed that no arg transform is executed.
            : m_Func(std::move(f)),
              m_Comparer(comparer),
              m_InputTransforms(inputTransformChain),
              m_MatchingArgumentIndices(matchingOutputIndices),
              m_OutputConstantTransformers(outputConstantTransformerPool),
              m_OutputVariableTransformers(outputVariableTransformerPool),
              m_LeftValueIndex(leftValueIndex),
              m_RightValueIndex(rightValueIndex),
              m_OutputTransformChainLength(outputTransformChainLength) {
            std::sort(m_InputTransforms.begin(), m_InputTransforms.end(), [](auto &left, auto &right) {
                return left->first < right->first;
            });

            if (m_MatchingArgumentIndices.size() != m_OutputVariableTransformers.size()) {
                throw std::invalid_argument(
                    "MRContext: Matching Argument Indices and Output variable transformer vector sizes must match. If there are Transformers without args, pass empty vector instead.");
            }

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

        bool ValidateTransformChains(const std::vector<std::any>& inputs, const size_t leftValueIndex, const size_t rightValueIndex,
                std::vector<Aya::MetamorphicRelation>& metamorphicRelations) override {
            bool match = false;
            // Create a sequence chain of some sorts?
            auto initialState = InvokeInternal(inputs, std::index_sequence_for<Args...>{});
            auto initialStateVector = MapTupleToVecNonVoid(initialState, std::index_sequence_for<Args...>{});
            auto followUpInputs = TransformInputs(inputs, std::index_sequence_for<Args...>{});
            auto followUpState = InvokeInternal(followUpInputs, std::index_sequence_for<Args...>{});
            auto followUpStateVec = MapTupleToVecNonVoid(followUpState, std::index_sequence_for<Args...>{});

            std::vector<std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>>> generatedOutputTransformChains;
            std::vector<std::shared_ptr<ITransformer>> totalOutputTransformerPool;
            totalOutputTransformerPool.reserve(m_OutputConstantTransformers.size());
            totalOutputTransformerPool.insert(totalOutputTransformerPool.end(), m_OutputConstantTransformers.begin(), m_OutputConstantTransformers.end());

            // Samples are the transformed outputs trying to match changes from initial to follow up
            if (m_BuildImplicitOutputTransforms) {
                auto variableOutputTransformers = ProduceVariableOutputTransformers(initialStateVector, m_RightValueIndex);
                totalOutputTransformerPool.insert(totalOutputTransformerPool.end(), variableOutputTransformers.begin(), variableOutputTransformers.end());
            }

            std::vector outputTransformIterators(m_OutputTransformChainLength, CartesianIterator({totalOutputTransformerPool.size()}));
            CompositeCartesianIterator outputTransformIterator(outputTransformIterators);
            while (!outputTransformIterator.isDone()) {
                std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformChain;
                auto pos = outputTransformIterator.getPos();
                for (auto &p: pos) {
                    for (const auto &i: p) {
                        auto pair = std::make_shared<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>(m_RightValueIndex, totalOutputTransformerPool[i]);
                        outputTransformChain.push_back(pair);
                    }
                }
                generatedOutputTransformChains.push_back(outputTransformChain);
                outputTransformIterator.next();
            }

            for (auto &outputTransformChain: generatedOutputTransformChains) {
                auto sampleOutput = TransformOutputs(initialStateVector, outputTransformChain);
                if (CompareTargetElements(std::any_cast<U>(sampleOutput[leftValueIndex]), std::any_cast<U>(followUpStateVec[rightValueIndex]))) {
                    metamorphicRelations.emplace_back(m_InputTransforms, outputTransformChain);
                    m_TotalMatches++;
                    match = true;
                }
            }

            return match;
        }

    private:
        std::function<T(Args...)> m_Func;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>> m_InputTransforms;
        std::vector<std::vector<size_t>> m_MatchingArgumentIndices; // double(double, int) => index 0 can be used to transform output 'double'. Generally specified by the user of the API
        std::function<bool(U, U)> m_Comparer;
        std::vector<std::shared_ptr<ITransformer>> m_OutputConstantTransformers;
        std::vector<std::shared_ptr<ITransformer>> m_OutputVariableTransformers;
        const size_t m_LeftValueIndex;
        const size_t m_RightValueIndex;
        const size_t m_OutputTransformChainLength;

        size_t m_TotalMatches = 0;

        bool m_BuildImplicitOutputTransforms = false;

        template<std::size_t... I>
        auto InvokeInternal(const std::vector<std::any>& inputs, std::index_sequence<I...>) {
            if constexpr (!std::is_void_v<T>) {
                auto args = std::make_tuple(std::any_cast<Args>(inputs[I])...);
                auto ret = m_Func(std::get<I>(args)...);
                return std::tuple_cat(std::make_tuple(ret), args);
            } else {
                auto args = std::make_tuple(std::any_cast<Args>(inputs[I])...);
                m_Func(std::get<I>(args)...);
                return args;
            }
        }

        std::vector<std::shared_ptr<ITransformer>> ProduceVariableOutputTransformers(std::vector<std::any>& stateVector, size_t targetOutputIndex) const {
            std::vector<std::shared_ptr<ITransformer>> newOutputTransformers;

            for (size_t i = 0; i < m_OutputVariableTransformers.size(); i++) {
                for (auto &index: m_MatchingArgumentIndices[i]) {
                    auto copp = m_OutputVariableTransformers[i]->Clone();
                    newOutputTransformers.push_back(copp);
                    newOutputTransformers.back()->OverrideArgs({stateVector[index+1]});
                    newOutputTransformers.back()->OverrideArgNames({"input[" + std::to_string(index) + "]"});
                }

            }
            return newOutputTransformers;
        }

        template<std::size_t... I>
        std::tuple<Args...> GetInputState(const std::vector<std::any>& inputs, std::index_sequence<I...>) {
            return std::make_tuple(std::any_cast<Args>(inputs[I])...);
        }

        template<std::size_t... I>
        std::tuple<ReturnType, Args...> GetOutputStateTuple(const std::vector<std::any>& inputs, std::index_sequence<I...>) {
            auto arg = std::make_tuple(std::any_cast<Args>(inputs[I+1])...);
            auto ret = std::make_tuple(std::any_cast<ReturnType>(inputs[0]));
            return std::tuple_cat(ret, arg);
        }

        template <std::size_t... I>
        std::vector<std::any> TransformInputs(const std::vector<std::any>& inputs, std::index_sequence<I...>) {
            auto inputsCopy(inputs);

            for (const auto &t: m_InputTransforms) {
                t->second->Apply(inputsCopy[t->first]);
            }
            return inputsCopy;
        }

        std::vector<std::any> TransformOutputs(const std::vector<std::any>& outputs,
            const std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>>& transformers) {
            auto outputsCopy(outputs);

            for (const auto &t: transformers) {
                t->second->Apply(outputsCopy[t->first]);
            }
            return outputsCopy;
        }

        std::string GetStateString(std::tuple<ReturnType, Args...> state) {
            return TupleToString(state);
        }

        template <size_t... I>
        std::vector<std::any> MapTupleToVecNonVoid(std::tuple<ReturnType, Args...> tup, std::index_sequence<I...>) {
            std::vector<std::any> result;
            result.push_back(std::any_cast<ReturnType>(std::get<0>(tup)));
            (result.push_back(std::get<I + 1>(tup)), ...);
            return result;
        }

        template <size_t... I>
        std::vector<std::any> MapTupleToVecVoid(std::tuple<Args...> tup, std::index_sequence<I...>) {
            std::vector<std::any> result;
            (result.push_back(std::get<I + 1>(tup)), ...);

            return result;
        }

        template <std::size_t... I>
        void PrintInputVec(const std::vector<std::any>& inputs, std::index_sequence<I...>) {
            auto tup = std::make_tuple(std::any_cast<Args>(inputs[I])...);
            std::cout << TupleToString(tup) << std::endl;
        }

        template <std::size_t... I>
        void PrintOutputVec(const std::vector<std::any>& inputs, std::index_sequence<I...>) {
            if constexpr (!std::is_void_v<T>) {
                auto r = std::make_tuple(std::any_cast<ReturnType>(inputs[0]));
                auto tup = std::make_tuple(std::any_cast<Args>(inputs[I+1])...);

                auto finalTup = std::tuple_cat(r, tup);
                std::cout << TupleToString(finalTup) << std::endl;
            } else {
                auto tup = std::make_tuple(std::any_cast<Args>(inputs[I])...);
                std::cout << TupleToString(tup) << std::endl;
            }
        }

        bool CompareTargetElements(const U e1, const U e2) {
            if (m_Comparer) {
                return m_Comparer(e1, e2);
            }

            return e1 == e2;
        }
    };
}
