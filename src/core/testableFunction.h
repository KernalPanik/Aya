#pragma once

#include "../common/tuple-utils.h"

#include <iostream>
#include <functional>
#include <tuple>
#include <optional>

class TestableFunctionBase {
public:
    virtual ~TestableFunctionBase() = default;
    virtual void* Invoke(void* args) = 0;
};

template<typename T, class... Args>
class TestableFunction : public TestableFunctionBase {
public:
    TestableFunction(std::function<T(Args...)> f) {
        this->func = f;
    }

    void* Invoke(void* args) override {
        if (std::is_void<T>().value) {
        } else {
            T r = InvokeImpl(std::index_sequence_for<Args...>{}, args);
            std::cout << r << std::endl;
        }
       return nullptr;
    }

private:
    std::function<T(Args&&... args)> func;

    template<std::size_t... I>
    T InvokeImpl(std::index_sequence<I...>, void* args) {
        std::tuple<Args...> concreteArgs = *static_cast<std::tuple<Args...>*>(args);
        std::cout << TupleToString(concreteArgs) << std::endl;
        T r = func(std::forward<Args>(std::get<I>(concreteArgs))...);
        return r;
    }
};


#pragma region Testable Function Interfaces
template<typename T, typename... Args, typename Callable>
std::shared_ptr<TestableFunctionBase> ConstructTestableFunction(Callable&& f) {
    return std::make_shared<TestableFunction<T, Args...>>(std::forward<Callable>(f));
}

template<class... Args>
std::shared_ptr<std::tuple<Args...>> InvokeTestableFunction(std::shared_ptr<TestableFunctionBase> func, Args&&... args) {
    auto t = std::make_shared<std::tuple<Args...>>(std::make_tuple(std::forward<Args>(args)...));
    func->Invoke(&t);
    return t;
}

#pragma endregion