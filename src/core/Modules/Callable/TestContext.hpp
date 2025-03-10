#pragma once

#include "src/Common/tuple-utils.h"
#include "transformer.h"

#include <functional>
#include <iostream>
#include <string>
#include <any>
#include <algorithm>

// TODO: 'Callable' is no longer a viable namespace for TestContext. Effectively, it is MR context from Aya 1 now. Move it to MRGen namespace?
namespace Callable {
    class ITestContext {
    public:
        virtual ~ITestContext() = default;
        virtual void TestInvoke() = 0;
        virtual void PrintState() = 0;
        virtual std::string ToString() = 0;
        virtual bool Equals(const std::shared_ptr<ITestContext>& other) = 0;
        virtual bool Equals(const std::string& other) = 0;
        virtual bool ValidateTransformChains(const std::vector<std::any>& inputs) = 0;
    };

    template <typename T, typename... Args>
    class TestContext final : public ITestContext {
    public:
        using ReturnType = std::conditional_t<
            std::is_void_v<T>,
            std::unique_ptr<int>,
            T>;

        // TODO: Make vector of transform chains definition shorter, probably switch to smaller sub-types with 'using'
        explicit TestContext(std::function<T(Args...)> f,
            const std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>& inputTransformChain,
            const std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>& outputTransformChain)
            :   m_Func(std::move(f)),
                m_InputTransforms(inputTransformChain),
                m_OutputTransforms(outputTransformChain) {
            std::sort(m_InputTransforms.begin(), m_InputTransforms.end(), [](auto &left, auto &right) {
                return left->first < right->first;
            });

            std::sort(m_OutputTransforms.begin(), m_OutputTransforms.end(), [](auto &left, auto &right) {
                return left->first < right->first;
            });
        }

        void TestInvoke() override {
            //InvokeInternal(std::index_sequence_for<Args...>{});
        }

        void PrintState() override {
            std::cout << ToString() << std::endl;
        }

        std::string ToString() override {
            /*if constexpr (!std::is_void_v<T>) {
                if (m_ReturnedValue != nullptr) {
                    std::tuple<T, std::decay_t<Args>...> finalState = std::tuple_cat(std::make_tuple(*m_ReturnedValue), m_ArgState);
                    return TupleToString(finalState);
                }
            }

            return TupleToString(m_ArgState);*/

            return std::string("TEST");
        }

        bool Equals(const std::shared_ptr<ITestContext> &other) override {
            return ToString().compare(other->ToString());
        }

        bool Equals(const std::string& other) override {
            return ToString().compare(other) == 0;
        }

        bool ValidateTransformChains(const std::vector<std::any>& inputs) override {
            std::cout << "Initial Inputs " << std::endl;
            auto initialState = GetInputState(inputs, std::index_sequence_for<Args...>{});
            std::cout << TupleToString(initialState) << std::endl;

            std::cout << "Initial Output State " << std::endl;
            auto status = InvokeInternal(inputs, std::index_sequence_for<Args...>{});
            std::cout << GetStateString(status) << std::endl;

            std::cout << "Applying Input Transforms... " << std::endl;
            auto followUpInputs = TransformInputs(inputs, std::index_sequence_for<Args...>{});
            std::cout << "Follow Up Inputs " << std::endl;
            PrintInputVec(followUpInputs, std::index_sequence_for<Args...>{});

            std::cout << "FollowUp Output State " << std::endl;
            auto followUpStatus = InvokeInternal(followUpInputs, std::index_sequence_for<Args...>{});
            std::cout << GetStateString(followUpStatus) << std::endl;

            std::cout << "Trying const transform on the output" << std::endl;
            auto followUpStateVec = MapTupleToVecNonVoid(followUpStatus, std::index_sequence_for<Args...>{});
            bool match = false;

            auto sampleOutput = TransformOutputs(followUpStateVec, std::index_sequence_for<Args...>{});

            std::cout << "FollowUp StateVec: " << std::endl;
            PrintOutputVec(followUpStateVec, std::index_sequence_for<Args...>{});
            std::cout << "Sample StateVec: " << std::endl;
            PrintOutputVec(sampleOutput, std::index_sequence_for<Args...>{});

            return match;
        }

    private:
        std::function<T(Args...)> m_Func;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> m_InputTransforms;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> m_OutputTransforms;

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

        template<std::size_t... I>
        std::tuple<Args...> GetInputState(const std::vector<std::any>& inputs, std::index_sequence<I...>) {
            return std::make_tuple(std::any_cast<Args>(inputs[I])...);
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

        auto TransformOutputState(std::tuple<ReturnType, Args...> state) {
            // TODO:
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
    };
}