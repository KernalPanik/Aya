#pragma once

#include <functional>

//TODO: Convert to proper interface (.hpp)

namespace Callable {
#pragma region Transformer Internals
    class BaseTransformer {
    public:
        virtual ~BaseTransformer() = default;
        virtual void Apply(void* data) = 0;
    };

    // Instance of a transfomer, applying a constant value
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

    // Instance of a transformer, applying a variable (controlled by an index in a state)
    // State is a tuple (result, x, y, ..., z)
    // Aim: Apply a transform on a specific index by transforming it with original state value at other index
    template <typename T, typename... Args>
    class VariableTransformer final : public BaseTransformer {
    public:
        void Apply(void* data) override {
            if (data == nullptr) {
                throw std::invalid_argument("Cannot transform base that is null.");
            }
        }

    private:
        std::function<void(T&, Args...)> func;
        std::tuple<Args...> args;
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

    // Maybe generate them on the fly, i.e. within the context?
    //TODO: Create an RFC for 'generation on the fly'
    // For codegen testing, or testing of functions where we only care about result value or a specific element in a final state, output transformation is enough to be the same as input
    // for elaborate state tracking, context-runtime transform search might be needed. Lets' consider it out of scope of "masters" version. VariableTransformer can be put as a 'prototype work'
    template<typename T, class... Args, typename Callable>
    std::shared_ptr<BaseTransformer> ConstructVariableTransformer(Callable&& f, Args&&... args) {
        return std::make_shared<VariableTransformer<T, Args...>>(std::forward<Callable>(f), std::forward<Args>(args)...);
    }

    /*
        Transformer Callables can be added to a vector and applied to a variable in bulk.
    */
    inline void ApplyTransformChain(void* base, const std::vector<std::shared_ptr<BaseTransformer>>& transformChain) {
        for (auto &transform : transformChain) {
            transform->Apply(base);
        }
    }

    struct TransformChain {
        std::vector<std::pair<size_t, std::shared_ptr<BaseTransformer>>> transforms;
    };

#pragma region TransformPool
    // Base function and applicable arguments, used to generate a list of transformers.
    // TODO: case when there is no U
    template <typename T, typename U>
    class TransformPool {
    public:
        std::function<void(T&, U)> func;
        std::vector<U> packedArgs;

        TransformPool(std::function<void(T&, U)> f, std::vector<U> vec)
            : func(f), packedArgs(vec) {}

        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<BaseTransformer>>>> GetTransformers(size_t index) {
            auto v = std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<BaseTransformer>>>>();
            
            for (auto &i : packedArgs) {
                
                auto t = ConstructTransformer<T, U>(func, std::decay_t<U>(i));
                v.push_back(std::make_shared<std::pair<size_t, std::shared_ptr<BaseTransformer>>>(std::make_pair(index, t)));
            }

            return v;
        }
    };
#pragma endregion
    template <typename T>
    std::vector<std::shared_ptr<BaseTransformer>> GetTransformersForType() {
        auto v = std::vector<std::shared_ptr<BaseTransformer>>();
        return v;
    }

    template <typename... Args>
    std::vector<std::shared_ptr<TransformChain>> ConstructPossibleTransformChains(size_t maxChainLength) {
        auto v = std::vector<std::shared_ptr<TransformChain>>();
        /*
            Iterate over args, get decltype() and look for specialization for a template.
            Template specialization is provided by the consumer.
            Produce vector of vectors for all variants, make dot product combination with length limit. include noop transformation for every type.
        */
        return v;
    }

#pragma endregion
}