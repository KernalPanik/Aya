#pragma once

#include "Caller.hpp"

struct IArgumentPack {
    virtual ~IArgumentPack() = default;
    virtual void* Apply(std::shared_ptr<ICaller> caller) = 0;

    virtual IArgumentPack Copy() = 0;
    virtual bool Copy(std::shared_ptr<IArgumentPack> other) = 0;
    virtual bool Print() = 0;
};

template <typename... Args>
struct ArgPack final : public IArgumentPack {
    std::tuple<Args...> tup;
    
    explicit ArgPack(Args&&... args) : tup(std::forward<Args>(args)...) {}

    void Apply(std::shared_ptr<ICaller> caller) {
        caller->Invoke();
        return nullptr;
    }
};


/*
    auto InvokeWithPackedArguments(const std::shared_ptr<TestableFunctionBase>& func, std::tuple<Args...>&& tupleOfArgs) {
        auto v = std::apply(
            [&](auto&&... args) {
                return TestableFunctionInvoker<T>(func, std::forward<decltype(args)>(args)...);
            },
            tupleOfArgs);

        return TupleDecay(*v);
    }
*/