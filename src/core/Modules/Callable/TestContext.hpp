#pragma once

#include "src/Common/tuple-utils.h"
#include "transformer.h"

#include <functional>
#include <iostream>
#include <string>
#include <any>

namespace Callable {
    class ITestContext {
    public:
        virtual ~ITestContext() = default;
        virtual void TestInvoke() = 0;
        virtual void PrintState() = 0;
        virtual std::string ToString() = 0;
        virtual bool Equals(const std::shared_ptr<ITestContext>& other) = 0;
        virtual bool Equals(const std::string& other) = 0;
        virtual bool ValidateTransformChains(
            std::shared_ptr<std::pair<size_t, std::shared_ptr<BaseTransformer>>> inputTransform,
            std::shared_ptr<std::pair<size_t, std::shared_ptr<BaseTransformer>>> outputTransform)= 0;
    };

    template <typename T, typename... Args>
    class TestContext final : public ITestContext {
    public:
        using ReturnType = std::conditional_t<
            std::is_void_v<T>,
            int,
            T>;

        explicit TestContext(std::function<T(Args...)> f, Args... args)
            : m_Func(std::move(f)),
            m_InitialArgState(static_cast<std::decay_t<Args>>(args)...),
            m_ArgState(static_cast<std::decay_t<Args>>(args)...) {}

        void TestInvoke() override {
            InvokeInternal(std::index_sequence_for<Args...>{});
        }

        void PrintState() override {
            std::cout << ToString() << std::endl;
        }

        std::string ToString() override {
            if constexpr (!std::is_void_v<T>) {
                if (m_ReturnedValue != nullptr) {
                    std::tuple<T, std::decay_t<Args>...> finalState = std::tuple_cat(std::make_tuple(*m_ReturnedValue), m_ArgState);
                    return TupleToString(finalState);
                }
            }

            return TupleToString(m_ArgState);
        }

        bool Equals(const std::shared_ptr<ITestContext> &other) override {
            return ToString().compare(other->ToString());
        }

        bool Equals(const std::string& other) override {
            return ToString().compare(other) == 0;
        }

        bool ValidateTransformChains(
            std::shared_ptr<std::pair<size_t, std::shared_ptr<BaseTransformer>>> inputTransform,
            std::shared_ptr<std::pair<size_t, std::shared_ptr<BaseTransformer>>> outputTransform) override {
            auto tmpTup = m_InitialArgState;

            size_t index = inputTransform->first;
            std::cout << TupleToString(tmpTup) << std::endl;
            std::cout << "Gonna apply transform to element at index " << index << std::endl;

            auto stateVec = TupleVec(tmpTup);
            inputTransform->second->Apply(stateVec[index]); // TODO: use size_t or int instead of rvalue const
            std::cout << TupleToString(tmpTup) << std::endl;

            // FOR OUTPUT TRANSFORMS:
            /*
             *  Try applying given constant transforms
             *  Construct new transforms with variables: Get args that can be used as transformer arguments...
             */

            return false;
        }

    private:
        std::function<T(Args...)> m_Func;
        std::tuple<std::decay_t<Args>...> m_InitialArgState;
        std::tuple<std::decay_t<Args>...> m_ArgState;
        std::shared_ptr<ReturnType> m_ReturnedValue;

        size_t m_TrackedOutputElement;
        std::vector<std::shared_ptr<BaseTransformer>> m_OutputTransformFunctions;
       // std::vector<std::any> m_OutputTransformConstants;

        template <std::size_t... I>
        void InvokeInternal(std::index_sequence<I...>) {
            if constexpr (std::is_void_v<T>) {
                m_Func(std::forward<Args>(std::get<I>(m_ArgState))...);
            } else {
                m_ReturnedValue = std::make_shared<ReturnType>(m_Func(std::forward<Args>(std::get<I>(m_ArgState))...));
            }
        }
    };
}