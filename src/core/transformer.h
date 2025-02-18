#pragma once

#include <functional>
#include <memory>

/*class BaseTransformer {
public:
    virtual ~BaseTransformer() = default;
    virtual void Apply() = 0;
};

template<class... Args>
class Transformer : BaseTransformer {
public:
    Transformer(std::function<void()> f) : func(f) {}

    void Apply(Args&&... args) override {
        func(std::forward<Args>(args)...);
    }

private:
    std::function<void(Args...)> func;
};
*/
template<class... Args>
class Transformer {
public:
    Transformer(std::function<void(Args...)> f) : func(f) {}

    void Apply(Args&&... args) {
        func(std::forward<Args>(args)...);
    }

private:
    std::function<void(Args...)> func;
};

// Base class for polymorphism
class BaseTransformer {
public:
    virtual ~BaseTransformer() = default;
    virtual void Apply() = 0;
};

// Derived class template to wrap Transformer and provide polymorphic behavior
template<class... Args>
class TransformerWrapper : public BaseTransformer {
public:
    TransformerWrapper(std::function<void(Args...)> f) : transformer(f) {}

    void Apply() override {
        transformer.Apply(2);
    }

private:
    Transformer<Args...> transformer;
};

template<class... Args, typename Callable>
std::shared_ptr<BaseTransformer> ConstructTransformer(Callable&& f) {
    // Deduce Args... from the callable signature
    return std::make_shared<TransformerWrapper<Args...>>(std::forward<Callable>(f));
}
