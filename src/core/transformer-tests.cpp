#include "transformer-tests.h"
//#include "transformer.h"

#include <iostream>
#include <string>

#include <iostream>
#include <functional>
#include <vector>
#include <memory>
#include <tuple>

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
        std::cout << "Calling Transformer Apply. Data was of size: " << sizeof(baseValue) << " Data itself is: " << baseValue << std::endl;
        ApplyImpl(std::index_sequence_for<Args...>{}, baseValue);
        std::cout << "Calling Transformer Apply. Data was of size: " << sizeof(baseValue) << " Data itself is: " << baseValue << std::endl;
        memcpy(data, &baseValue, sizeof(baseValue));
    }

private:
    template<std::size_t... I>
    void ApplyImpl(std::index_sequence<I...>, T& baseValue) {
        func(baseValue, std::forward<Args>(std::get<I>(args))...);
    }

    std::function<void(T&, Args...)> func;  // The stored function to apply
    std::tuple<Args...> args;  // Arguments to pass to the function (excluding the base argument)
};

// Constructor wrapper function, accepting a callable (lambda or function pointer)
template<typename T, class... Args, typename Callable>
std::shared_ptr<BaseTransformer> ConstructTransformer(Callable&& f, Args&&... args) {
    return std::make_shared<Transformer<T, Args...>>(std::forward<Callable>(f), std::forward<Args>(args)...);
}

// Example function that modifies the base argument (T&)
void ModifyBase(int& base, std::string check) {
    std::cout << "Before: " << base << ", " << check << std::endl;
    base *= 2;  // Modify the base argument
    std::cout << "After: " << base << std::endl;
}

void TransformerTests_SimpleCall() {
    auto baseValue = 10;
    auto v = 0;

    // Define a vector of pointers to BaseTransformer
    std::vector<std::shared_ptr<BaseTransformer>> transformers;

    // Create a Transformer for ModifyBase with int& as base argument and string argument
    auto t1 = ConstructTransformer<int, std::string>(ModifyBase, "Hello, World!");  // No need to pass baseValue here

    // Add it to the vector
    transformers.push_back(t1);

    // Call Apply on each Transformer in the vector, passing the baseValue
    transformers[0]->Apply(&baseValue);  // Transforms baseValue

    std::cout << "Final base value: " << baseValue << std::endl;
}

void TransformerTests_MutableCall() {}
void TransformerTests_Various() {}