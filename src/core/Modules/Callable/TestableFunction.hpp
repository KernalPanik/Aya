#pragma once

#include "TestableFunction.hpp"
#include "src/Common/tuple-utils.h"

#include <functional>
#include <iostream>
#include <string>

using namespace Callable;

class ITestableFunction {
public:
    virtual ~ITestableFunction() = default;
    virtual void Invoke() = 0;
    virtual void PrintState() = 0;
    virtual std::string ToString() = 0;
    virtual bool Equals(std::shared_ptr<ITestableFunction> other) = 0;
    virtual bool Equals(std::string other) = 0;
};

//NOTE: maybe rename it to context after all?
template <typename T, typename... Args>
class TestableFunction final : public ITestableFunction {
public:
    using ReturnType = std::conditional_t<
        std::is_void_v<T>,
        int,
        T>;
  
    explicit TestableFunction(std::function<T(Args...)> f, Args&&... args)
        : m_Func(std::move(f)), m_ArgState(static_cast<std::decay_t<Args>>(args)...) {}

    void Invoke() override {
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

    bool Equals(std::shared_ptr<ITestableFunction> other) override {
        return ToString().compare(other->ToString());
    }

    bool Equals(std::string other) override {
        return ToString().compare(other);
    }

private:
    std::function<T(Args...)> m_Func;
    std::tuple<std::decay_t<Args>...> m_ArgState;
    std::shared_ptr<ReturnType> m_ReturnedValue;

    template <std::size_t... I>
    void InvokeInternal(std::index_sequence<I...>) {
        if constexpr (std::is_void_v<T>) {
            m_Func(std::forward<Args>(std::get<I>(m_ArgState))...);
        } else {
            m_ReturnedValue = std::make_shared<ReturnType>(m_Func(std::forward<Args>(std::get<I>(m_ArgState))...));
        }
    }
};