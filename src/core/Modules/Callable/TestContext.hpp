#pragma once

#include "src/Common/tuple-utils.h"
#include "transformer.h"

#include <functional>
#include <iostream>
#include <string>
#include <any>

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
        virtual bool ValidateTransformChains(std::vector<std::any> inputs) = 0;
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
            const std::shared_ptr<std::pair<size_t, std::vector<std::shared_ptr<ITransformer>>>>& inputTransformChain,
            const std::shared_ptr<std::pair<size_t, std::vector<std::shared_ptr<ITransformer>>>>& outputTransformChain)
            :   m_Func(std::move(f)),
                m_InputTransforms(inputTransformChain),
                m_OutputTransforms(outputTransformChain) {}

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

        /*  USAGE SCENARIO:
         * - initialize a chain of possible input transformers
         * - initialize a chain of possible output transformers
         * - Iterate over combinations of i/o transformers
         * - for each validation, generate additional VariableTransformers operating on given variables instead of constants
         * - return true if there is at least one MR, fill the MR vector, allow it to be retrieved from the context.
         */
        // TODO: Revert back to the idea of having a ctx with one input and output TC, and validate them against given input (still pass inputs as vectors of any)
        bool ValidateTransformChains(std::vector<std::any> inputs) override {
            for (auto& input: inputs) {

            }
            return false;
        }

    private:
        std::function<T(Args...)> m_Func;
        std::shared_ptr<std::pair<size_t, std::vector<std::shared_ptr<ITransformer>>>> m_InputTransforms;
        std::shared_ptr<std::pair<size_t, std::vector<std::shared_ptr<ITransformer>>>> m_OutputTransforms;

        /*std::tuple<std::decay_t<Args>...> m_InitialArgState;
        std::tuple<std::decay_t<Args>...> m_ArgState;

        std::shared_ptr<ReturnType> m_InitialReturnedValue;
        std::unique_ptr<ReturnType> m_ReturnedValue;

        // Used to expose tested function to a consumer
        template <std::size_t... I>
         void InvokeInternal(std::index_sequence<I...>) {
            if constexpr (std::is_void_v<T>) {
                m_Func(std::forward<Args>(std::get<I>(m_ArgState))...);
            } else {
                m_ReturnedValue = std::make_unique<ReturnType>(m_Func(std::forward<Args>(std::get<I>(m_ArgState))...));
            }
        }

        std::tuple<ReturnType, std::decay_t<Args>...> InvokeAndGetState() {
            if constexpr (std::is_void_v<T>) {
                m_ReturnedValue = nullptr;
                m_Func(std::forward<Args>(m_ArgState)...);
                return std::tuple_cat(std::make_tuple(nullptr), m_ArgState);
            } else {
                m_ReturnedValue = std::make_unique<ReturnType>(m_Func(std::forward<Args>(m_ArgState)...));
                return std::tuple_cat(std::make_tuple(m_ReturnedValue), m_ArgState);
            }
        }*/

        void reportMessage(std::string msg, bool verbose) {
            if (verbose) {
                std::cout << msg << std::endl;
            }
        }
    };
}