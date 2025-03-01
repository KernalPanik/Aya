#pragma once

#include "TestableFunction.hpp"
#include "src/Common/tuple-utils.h"

#include <functional>

using namespace Callable;

class ITestableFunction {
public:
    virtual ~ITestableFunction() = default;
    virtual void Invoke() = 0;
    virtual void PrintState() = 0;
    virtual bool CompareStates(std::shared_ptr<ITestableFunction> other) = 0;
};


//NOTE: maybe rename it to context after all?
template <typename T, typename... Args>
class TestableFunction final : public ITestableFunction {
public:
    using StateType = std::conditional_t<
        std::is_void_v<T>,
        std::tuple<Args...>,
        std::tuple<T, Args...>>;

    explicit TestableFunction(std::function<T(Args...)> f, Args&&... args)
        : m_Func(std::move(f)), m_InitialState(args...) {}

    void Invoke() override {
        //m_Func->Invoke();
    }

    void PrintState() override {
        if (m_State != nullptr) {
            std::cout << TupleToString(*m_State) << std::endl;
        } else {
            std::cout << TupleToString(m_InitialState) << std::endl;
        }
    }

    bool CompareStates(std::shared_ptr<ITestableFunction> other) override {
      //TODO: Need to somehow extract the tuple values -- getAtIndex()?
      return false;
    }

private:
    std::function<T(Args&&...)> m_Func;
    std::tuple<Args...> m_InitialState;
    std::shared_ptr<StateType> m_State;
};