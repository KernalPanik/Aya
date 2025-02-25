#pragma once

#include "metamorphicRelation.h"
#include "Modules/Callable/testableFunction.h"

using namespace Callable;

template <typename T, class... Args>
void SearchForMRs(TestableFunction<T, Args...> testableFunc, MetamorphicRelation* mrs, size_t& mrCount) { }

// Type-erased TestableFunction
/*
// Base class for type-erasure
class TestableFunctionBase {
public:
    virtual ~TestableFunctionBase() = default;
    
    // A pure virtual function that all derived classes must implement.
    virtual std::tuple<void*> Invoke(void* args...) = 0;
};

// Derived class that holds the actual implementation of the templated function.
template <typename T, class... Args>
class TestableFunctionImpl : public TestableFunctionBase {
public:
    TestableFunctionImpl(std::function<T(Args&&...)> f) : func(std::move(f)) {}

    std::tuple<void*> Invoke(void* args...) override {
        return invokeImpl(std::index_sequence_for<Args...>{}, std::forward<void*>(args)...);
    }

private:
    std::function<T(Args&&...)> func;

    template <std::size_t... I>
    std::tuple<void*> invokeImpl(std::index_sequence<I...>, void* args...) {
        return std::make_tuple((void*) &func(std::forward<Args>(*static_cast<Args*>(args)))...);
    }
};

// Type-erased wrapper
class TestableFunction {
public:
    template <typename T, class... Args>
    TestableFunction(std::function<T(Args&&...)> f) {
        impl = std::make_shared<TestableFunctionImpl<T, Args...>>(std::move(f));
    }

    // Call Invoke on the wrapped function
    std::tuple<void*> Invoke(void* args...) {
        return impl->Invoke(args...);
    }

private:
    std::shared_ptr<TestableFunctionBase> impl;
};



 */