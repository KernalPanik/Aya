#pragma once

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
class TestableFunctionV2 : public TestableFunctionBase {
public:
    TestableFunctionV2(std::function<T(Args...)> f) {
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
        return func(std::forward<Args>(std::get<I>(concreteArgs))...);
    }
};

template<typename T, typename... Args, typename Callable>
std::shared_ptr<TestableFunctionBase> ConstructTestableFunction(Callable&& f) {
    return std::make_shared<TestableFunctionV2<T, Args...>>(std::forward<Callable>(f));
}

template<class... Args>
void* InvokeTestableFunction(std::shared_ptr<TestableFunctionBase> func, Args&&... args) {
    auto t = std::make_tuple(std::forward<Args>(args)...);
    return func->Invoke(&t);
}