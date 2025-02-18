#pragma once

#include <functional>
#include <memory>

class BaseTransformer {
public:
    virtual ~BaseTransformer() = default;
    virtual void Apply(void* data) = 0;
};

template<typename T, class... Args>
class Transformer : public BaseTransformer {
public:
    Transformer(std::function<void(T&, Args...)> f, Args&&... args)
        : func(f), args(std::make_tuple(std::forward<Args>(args)...)) {}

    void Apply(void* data) override {
        auto baseValue = *reinterpret_cast<T*>(data);
        ApplyImpl(std::index_sequence_for<Args...>{}, baseValue);
        memcpy(data, &baseValue, sizeof(baseValue));
    }

private:
    template<std::size_t... I>
    void ApplyImpl(std::index_sequence<I...>, T& baseValue) {
        func(baseValue, std::forward<Args>(std::get<I>(args))...);
    }

    std::function<void(T&, Args...)> func;
    std::tuple<Args...> args;
};

template<typename T, class... Args, typename Callable>
std::shared_ptr<BaseTransformer> ConstructTransformer(Callable&& f, Args&&... args) {
    return std::make_shared<Transformer<T, Args...>>(std::forward<Callable>(f), std::forward<Args>(args)...);
}

inline void ApplyTransformChain(void* val, std::vector<std::shared_ptr<BaseTransformer>> transformChain) {
    for (auto &tc : transformChain) {
        tc->Apply(val);
    }
}