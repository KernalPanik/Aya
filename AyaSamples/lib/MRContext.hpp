#pragma once

#include "tuple-utils.h"
#include "transformer.hpp"
#include "TransformBuilder.hpp"
#include "MetamorphicRelation.hpp"

#include <functional>
#include <iostream>
#include <string>
#include <any>
#include <algorithm>

namespace Aya {
    class IMRContext {
    public:
        virtual ~IMRContext() = default;
        // std::vector<std::any> is an abstract class friendly way of having a tuple. So MRSearch would accept a vector of vectors, encompassing inputs.
        virtual bool ValidateTransformChains(const std::vector<std::any>& inputs, size_t targetOutputIndex, std::vector<Aya::MetamorphicRelation>& metamorphicRelations) = 0;
        [[nodiscard]]
        virtual size_t GetTotalMatches() const = 0;
        virtual void OverrideComparerMethod(std::any func) = 0;
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
            const std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>>& inputTransformChain,
            const std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>>& outputTransformChain,
            std::vector<std::function<void(T&, T)>> variableOutputTransformingFunctions,
            std::vector<std::string> variableOutputTransformFunctionNames,
            const std::vector<size_t>& matchingVariableTransformingIndices)
            :   m_Func(std::move(f)),
                m_InputTransforms(inputTransformChain),
                m_OutputTransforms(outputTransformChain),
                m_OutputTransformFuncs(variableOutputTransformingFunctions),
                m_OutputTransformFuncNames(std::move(variableOutputTransformFunctionNames)),
                m_MatchingArgumentIndices(matchingVariableTransformingIndices) {
            std::sort(m_InputTransforms.begin(), m_InputTransforms.end(), [](auto &left, auto &right) {
                return left->first < right->first;
            });

            std::sort(m_OutputTransforms.begin(), m_OutputTransforms.end(), [](auto &left, auto &right) {
                return left->first < right->first;
            });

            m_TotalMatches = 0;
            m_BuildImplicitOutputTransforms = false;
        }

        [[nodiscard]]
        size_t GetTotalMatches() const override {
            return m_TotalMatches;
        }

        // Might be crap for C# interface...
        // If so, pass raw function pointer, and convert it to std::function
        void OverrideComparerMethod(std::any func) override {
            m_Comparer = std::make_unique<std::function<bool(U, U)>>(std::any_cast<std::function<bool(U, U)>>(func));
        }

        void SetImplicitOutputTransforms(const bool value) override {
            m_BuildImplicitOutputTransforms = value;
        }

        bool ValidateTransformChains(const std::vector<std::any>& inputs, const size_t targetOutputIndex,
                std::vector<Aya::MetamorphicRelation>& metamorphicRelations) override {
            bool match = false;
            // Create a sequence chain of some sorts?
            auto initialState = InvokeInternal(inputs, std::index_sequence_for<Args...>{});
            auto initialStateVector = MapTupleToVecNonVoid(initialState, std::index_sequence_for<Args...>{});
            auto followUpInputs = TransformInputs(inputs, std::index_sequence_for<Args...>{});
            auto followUpState = InvokeInternal(followUpInputs, std::index_sequence_for<Args...>{});
            auto followUpStateVec = MapTupleToVecNonVoid(followUpState, std::index_sequence_for<Args...>{});

            // Samples are the transformed outputs trying to match changes from initial to follow up
            auto sampleOutput = TransformOutputs(initialStateVector, std::index_sequence_for<Args...>{});
            auto sampleOutputTuple = GetOutputStateTuple(sampleOutput, std::index_sequence_for<Args...>{});

            if (CompareTargetElements(std::any_cast<U>(followUpStateVec[targetOutputIndex]), std::any_cast<U>(sampleOutput[targetOutputIndex]))) {
                metamorphicRelations.emplace_back(m_InputTransforms, m_OutputTransforms);
                m_TotalMatches++;
                match = true;
            }

            if (m_BuildImplicitOutputTransforms) {
                std::vector<std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>>> producedOutputTransforms;
                auto variableTransformedOutputSamples = ApplyVariableOutputTransforms(initialStateVector, targetOutputIndex, producedOutputTransforms);
                size_t index = 0;
                for (auto &sample : variableTransformedOutputSamples) {
                    auto sampleTup = GetOutputStateTuple(sample, std::index_sequence_for<Args...>{});
                    auto state1 = MapTupleToVecNonVoid(followUpState, std::index_sequence_for<Args...>{});

                    auto el1 = state1[targetOutputIndex];
                    auto el2 = sample[targetOutputIndex];

                    if (CompareTargetElements(std::any_cast<U>(el1), std::any_cast<U>(el2))) {
                        metamorphicRelations.emplace_back(m_InputTransforms, producedOutputTransforms[index]);
                        m_TotalMatches++;
                    }
                    index++;
                    match = true;
                }
            }

            return match;
        }

    private:
        std::function<T(Args...)> m_Func;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>> m_InputTransforms;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>> m_OutputTransforms;
        // Accept only base func case for now. Not yet sure how to make it nicely expansible, but simple func should be enough for current experiments within the Master's
        // TODO: Maybe specify this type separately, like U.
        std::vector<std::function<void(U&, U)>> m_OutputTransformFuncs;
        std::vector<std::string> m_OutputTransformFuncNames;
        std::vector<size_t> m_MatchingArgumentIndices; // double(double, int) => index 0 can be used to transform output 'double'
        std::unique_ptr<std::function<bool(U, U)>> m_Comparer;

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

        // Accepts final state, i.e. with an output, if exists.
        // TODO: Use special type to control outputs, like S or U
        // TODO: Make it work properly with void return type too, maybe add tests.
        std::vector<std::vector<std::any>> ApplyVariableOutputTransforms(std::vector<std::any>& stateVector,
                const size_t targetOutputIndex,
                std::vector<std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>>>& producedOutputTransformChains) {
            auto stateCopy(stateVector);
            size_t offset = 0;
            if constexpr (!std::is_void_v<T>) {
                offset++; // operating on arguments. 0'th argument is now first, and so on.
            }

            std::vector<T> matchingArgs;
            matchingArgs.reserve(m_MatchingArgumentIndices.size());
            for (auto &index : m_MatchingArgumentIndices) {
                matchingArgs.push_back(std::any_cast<T>(stateVector[index+offset]));
            }

            auto argNameOverrides = std::vector<std::string>();
            for (const auto &index : m_MatchingArgumentIndices) {
                argNameOverrides.push_back("input" + std::to_string(index));
            }

            std::vector<std::vector<std::any>> result;
            for (size_t j = 0; j < m_OutputTransformFuncs.size(); j++) {
                for (size_t i = 0; i < m_MatchingArgumentIndices.size(); i++) {
                    auto matchingArgOverrides = std::vector<std::string>();
                    matchingArgOverrides.emplace_back(argNameOverrides[i]);
                    auto transformer = Aya::TransformBuilder<U, U>().MapTransformersToStateIndex(m_OutputTransformFuncs[j], m_OutputTransformFuncNames[j], {matchingArgs[i]}, targetOutputIndex, matchingArgOverrides);
                    auto newState = TransformOutputs(stateCopy, transformer, std::index_sequence_for<Args...>{});
                    producedOutputTransformChains.push_back(transformer);
                    result.push_back(newState);
                }
            }

            return result;
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

        template <std::size_t... I>
        std::vector<std::any> TransformOutputs(const std::vector<std::any>& outputs, std::index_sequence<I...>) {
            auto outputsCopy(outputs);

            for (const auto &t: m_OutputTransforms) {
                t->second->Apply(outputsCopy[t->first]);
            }
            return outputsCopy;
        }

        template <std::size_t... I>
        std::vector<std::any> TransformOutputs(const std::vector<std::any>& outputs,
            const std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<Aya::ITransformer>>>>& transformers, std::index_sequence<I...>) {
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
            if (m_Comparer != nullptr) {
                return (*m_Comparer)(e1, e2);
            }

            return e1 == e2;
        }
    };
}