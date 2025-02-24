#pragma once

#include "../common/tuple-utils.h"

#include <iostream>
#include <functional>
#include <tuple>
#include <optional>
#include <any>

struct TupleWrapperBase {
    virtual ~TupleWrapperBase() = default;
};

template <typename ... Args>
struct TupleWrapper : public TupleWrapperBase {
    std::tuple<Args...> tup;
    explicit TupleWrapper(Args&&... args) : tup(std::forward<Args>(args)...) {}
};

// returning shared_ptr looks nice on paper, however, I wonder how will this look like in a context of C# interface.
// For now, I'll take a risk and use smart pointers as much as possible, providing raw pointers only at the C++ to C# level.
class TestableFunctionBase {
public:
    virtual ~TestableFunctionBase() = default;
    virtual std::shared_ptr<void> Invoke(TupleWrapperBase* args) = 0;
};

template<typename T, class... Args>
class TestableFunction final : public TestableFunctionBase {
public:
    explicit TestableFunction(std::function<T(Args...)> f) {
        this->func = f;
    }

    std::shared_ptr<void> Invoke(TupleWrapperBase* args) override {
        auto* concreteArgs = dynamic_cast<TupleWrapper<Args...>*>(args);
        if (!concreteArgs) {
            throw std::runtime_error("Invalid tuple type");
        }

        if (std::is_void<T>().value) {
            InvokeImpl(std::index_sequence_for<Args...>{}, concreteArgs->tup);
        } else {
            T r = InvokeImpl(std::index_sequence_for<Args...>{}, concreteArgs->tup);
            return std::make_shared<T>(r);
        }
       return nullptr;
    }

private:
    std::function<T(Args&&... args)> func;

    template<std::size_t... I>
    T InvokeImpl(std::index_sequence<I...>, std::tuple<Args...>& tup) {
        T r = func(std::forward<Args>(std::get<I>(tup))...);
        return r;
    }
};

#pragma region Testable Function Interfaces
template<typename T, typename... Args, typename Callable>
std::shared_ptr<TestableFunctionBase> ConstructTestableFunction(Callable&& f) {
    return std::make_shared<TestableFunction<T, Args&&...>>(
        std::function<T(Args&&...)>(std::forward<Callable>(f)));
}

template<typename T,
class... Args,
typename = std::enable_if_t<!std::is_void_v<T>>>
auto InvokeTestableFunction(const std::shared_ptr<TestableFunctionBase>& func, Args&&... args) {
    auto wrapper = std::make_unique<TupleWrapper<Args...>>(std::forward<Args>(args)...);
    auto* concreteArgs = dynamic_cast<TupleWrapper<Args...>*>(wrapper.get());

    auto retvalPtr = func->Invoke(wrapper.get());
    if (retvalPtr) {
        T val = *static_cast<T*>(retvalPtr.get());
        return std::make_shared<std::tuple<T, Args...>>(val, std::forward<Args>(args)...);
    }

    throw std::logic_error("Expected some return from non void function!\n");
}


// TODO: Specialization for void func
/*
template<typename T,
class... Args,
typename = std::enable_if_t<!std::is_void_v<T>>>
std::shared_ptr<std::tuple<T, Args...>> InvokeTestableFunction(const std::shared_ptr<TestableFunctionBase>& func, Args&&... args) {
    auto wrapper = std::make_unique<TupleWrapper<Args...>>(std::forward<Args>(args)...);
    auto* concreteArgs = dynamic_cast<TupleWrapper<Args...>*>(wrapper.get());

    auto retvalPtr = func->Invoke(wrapper.get());
    if (retvalPtr) {
        T val = *static_cast<T*>(retvalPtr.get());
        return std::make_shared<std::tuple<T, Args...>>(val, std::forward<Args>(args)...);
    }

    throw std::logic_error("Expected some return from non void function!\n");
}*/


template <typename T, typename... Args>
auto TestableFunctionInvoker(const std::shared_ptr<TestableFunctionBase>& func, Args&&... args) {
    return InvokeTestableFunction<T>(func, std::forward<decltype(args)>(args)...);
}


// Returns decayed (only values) tuple describing final function state
// This is the preferred way to interact with TestableFunction
template<typename T, typename... Args>
auto InvokeWithPackedArguments(const std::shared_ptr<TestableFunctionBase>& func, std::tuple<Args...>&& tupleOfArgs) {
    auto v = std::apply(
        [&](auto&&... args) {
            return TestableFunctionInvoker<T>(func, std::forward<decltype(args)>(args)...);
        },
        tupleOfArgs);

    return TupleDecay(*v);
}

#pragma endregion