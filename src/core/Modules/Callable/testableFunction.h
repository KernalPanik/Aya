#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <tuple>

#include "src/Common/tuple-utils.h"

namespace Callable {
    #pragma region Testable Function Internals
   /*
    // returning shared_ptr looks nice on paper, however, I wonder how will this look like in a context of C# interface.
    // For now, I'll take a risk and use smart pointers as much as possible, providing raw pointers only at the C++ to C# level.
    class TestableFunctionBase {
    public:
        virtual ~TestableFunctionBase() = default;
        virtual std::shared_ptr<void> Invoke(TupleWrapperBase* args) = 0;
    };

    struct TupleWrapperBase {
        virtual ~TupleWrapperBase() = default;
        virtual void ApplyToFunc(const std::shared_ptr<TestableFunctionBase>& func) = 0;
    };

    template <typename... Args>
    struct TupleWrapper final : TupleWrapperBase {
        std::tuple<Args...> tup;
        explicit TupleWrapper(Args&&... args) : tup(std::forward<Args>(args)...) {}

        void ApplyToFunc(const std::shared_ptr<TestableFunctionBase>& func) override {
            auto v = std::apply(
                [&](auto&&... args) {
                    return TestableFunctionInvoker<T>(func, std::forward<decltype(args)>(args)...);
                },
                tup);

            return TupleDecay(*v);
        }
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

            return InvokeImpl(std::index_sequence_for<Args...>{}, concreteArgs->tup);
        }

    private:
        std::function<T(Args&&... args)> func;

        template <std::size_t... I>
        std::shared_ptr<T> InvokeImpl(std::index_sequence<I...>, std::tuple<Args...>& tup) {
            if constexpr (!std::is_void_v<T>) {
                T r = func(std::forward<Args>(std::get<I>(tup))...);
                return std::make_shared<T>(r);
            }

            func(std::forward<Args>(std::get<I>(tup))...);
            return nullptr;
        }
    };

    template <typename T, typename... Args>
    auto TestableFunctionInvoker(const std::shared_ptr<TestableFunctionBase>& func, Args&&... args) {
        return InvokeTestableFunction<T>(func, std::forward<decltype(args)>(args)...);
    }
    */
    #pragma endregion

    #pragma region Testable Function Interfaces
  /*
  
    template<typename T,
    class... Args,
    typename = std::enable_if_t<!std::is_void_v<T>>>
    auto InvokeTestableFunction(const std::shared_ptr<TestableFunctionBase>& func, Args&&... args) {
        auto wrapper = std::make_unique<TupleWrapper<Args...>>(std::forward<Args>(args)...);
        auto* concreteArgs = dynamic_cast<TupleWrapper<Args...>*>(wrapper.get());

        if (auto retvalPtr = func->Invoke(wrapper.get())) {
            T val = *static_cast<T*>(retvalPtr.get());
            return std::make_shared<std::tuple<T, Args...>>(val, std::forward<Args>(args)...);
        }

        throw std::logic_error("Expected some return from non void function!\n");
    }

    template <typename T,
    class... Args,
    typename = std::enable_if_t<std::is_void_v<T>>>
    std::shared_ptr<std::tuple<Args...>> InvokeTestableFunction(const std::shared_ptr<TestableFunctionBase>& func, Args&&... args) {
        auto wrapper = std::make_unique<TupleWrapper<Args...>>(std::forward<Args>(args)...);
        auto* concreteArgs = dynamic_cast<TupleWrapper<Args...>*>(wrapper.get());

        auto retvalPtr = func->Invoke(wrapper.get());
        if (!retvalPtr) {
            return std::make_shared<std::tuple<Args...>>(std::forward<Args>(args)...);
        }

        throw std::logic_error("Expected some return from non void function!\n");
    }
*/
    /*
        Main function to construct an interface to a TestableFunction.
        Usage:
        const auto testableFunction = ConstructTestableFunction<void, int&>(nonStateChangingVoidFunc);
            - First Template argument is the return type (can be void too).
            - Following arguments are function arguments passed as references, even if original function does not need a reference here.
        Invoke the testableFunction using InvokeWithPackedArguments (Preferred way). Or use direct calls to InvokeTestableFunction()
    */
  /*  template <typename T, typename... Args, typename Callable>
    std::shared_ptr<TestableFunctionBase> ConstructTestableFunction(Callable&& f) {
        return std::make_shared<TestableFunction<T, Args&&...>>(
            std::function<T(Args&&...)>(std::forward<Callable>(f)));
    }
*/
    /*
        Main function to invoke a TestableFunction
        Expects a tuple of arguments, not a vararg.
        Usage:
        const auto testableFunction = ConstructTestableFunction<void, int&>(testFunc);
        auto packedInputs = std::make_tuple(static_cast<int>(42));
        const auto finalState = InvokeWithPackedArguments<void>(testableFunction, std::move(packedInputs));

        Make sure to always std::move the tuple

        Returns decayed (only values) tuple describing final function state
    */

    //TODO: make invoker call apply function defined in TupleWrapper
    /*
        The Apply function must take the instance of an invoker with a type, and apply tuple to it.
     */
 /*
    template<typename T, typename... Args>
    auto InvokeWithPackedArguments(const std::shared_ptr<TestableFunctionBase>& func, std::tuple<Args...>&& tupleOfArgs) {
        auto v = std::apply(
            [&](auto&&... args) {
                return TestableFunctionInvoker<T>(func, std::forward<decltype(args)>(args)...);
            },
            tupleOfArgs);

        return TupleDecay(*v);
    }
*/
    #pragma endregion
}