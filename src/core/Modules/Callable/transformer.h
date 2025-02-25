#pragma once

#include <functional>

namespace Callable {
#pragma region Transformer Internals
    class BaseTransformer {
    public:
        virtual ~BaseTransformer() = default;
        virtual void Apply(void* data) = 0;
    };

    template<typename T, class... Args>
    class Transformer final : public BaseTransformer {
    public:
        explicit Transformer(std::function<void(T&, Args...)> f, Args&&... args)
            : func(f), args(std::make_tuple(std::forward<Args>(args)...)) {}

        void Apply(void* data) override {
            if (data == nullptr) {
                throw std::invalid_argument("Cannot transform base that is null.");
            }

            auto baseValue = *static_cast<T*>(data);
            ApplyImpl(std::index_sequence_for<Args...>{}, baseValue);
            memcpy(data, &baseValue, sizeof(baseValue));
        }

    private:
        std::function<void(T&, Args...)> func;
        std::tuple<Args...> args;

        template<std::size_t... I>
        void ApplyImpl(std::index_sequence<I...>, T& baseValue) {
            func(baseValue, std::forward<Args>(std::get<I>(args))...);
        }
    };
#pragma endregion

#pragma region Transformer Interfaces

    /*
        Constructor for a Transformer Object. Transformer is an encapsulation of
        std::function<> packaged with constant arguments. For example, a function that constantly multiplies the value by 5.

        This is the preferable way to interact with the Transformer Callable.
        example:
        const auto transformer = ConstructTransformer<int>(IntTransformingFunction);
        int x;
        transformer->Apply(x);
    */
    template<typename T, class... Args, typename Callable>
    std::shared_ptr<BaseTransformer> ConstructTransformer(Callable&& f, Args&&... args) {
        return std::make_shared<Transformer<T, Args...>>(std::forward<Callable>(f), std::forward<Args>(args)...);
    }

    /*
        Transformer Callables can be added to a vector and applied to a variable in bulk.
    */
    inline void ApplyTransformChain(void* base, const std::vector<std::shared_ptr<BaseTransformer>>& transformChain) {
        for (auto &transform : transformChain) {
            transform->Apply(base);
        }
    }
#pragma endregion
}