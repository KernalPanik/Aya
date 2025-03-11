#pragma once

#include <functional>
#include <any>

//TODO: Convert to proper interface (.hpp)

namespace Callable {
#pragma region Transformer Internals
    class ITransformer {
    public:
        virtual ~ITransformer() = default;
        virtual void Apply(void* data) = 0;
        virtual void Apply(std::any& data) = 0;
    };

    // Instance of a transfomer, applying a constant value
    template<typename T, class... Args>
    class Transformer final : public ITransformer {
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

        void Apply(std::any& data) override {
            auto baseValue = std::any_cast<T>(data);
            ApplyImpl(std::index_sequence_for<Args...>{}, baseValue);
            data = baseValue;
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
    class VariableTransformer final : public ITransformer {
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
    std::shared_ptr<ITransformer> ConstructTransformer(Callable&& f, Args&&... args) {
        return std::make_shared<Transformer<T, Args...>>(std::forward<Callable>(f), std::forward<Args>(args)...);
    }

    // Maybe generate them on the fly, i.e. within the context?
    //TODO: Create an RFC for 'generation on the fly'
    // For codegen testing, or testing of functions where we only care about result value or a specific element in a final state, output transformation is enough to be the same as input
    // for elaborate state tracking, context-runtime transform search might be needed. Lets' consider it out of scope of "masters" version. VariableTransformer can be put as a 'prototype work'
    template<typename T, class... Args, typename Callable>
    std::shared_ptr<ITransformer> ConstructVariableTransformer(Callable&& f, Args&&... args) {
        return std::make_shared<VariableTransformer<T, Args...>>(std::forward<Callable>(f), std::forward<Args>(args)...);
    }

    /*
        Transformer Callables can be added to a vector and applied to a variable in bulk.
    */
    inline void ApplyTransformChain(void* base, const std::vector<std::shared_ptr<ITransformer>>& transformChain) {
        for (auto &transform : transformChain) {
            transform->Apply(base);
        }
    }

    struct TransformChain {
        std::vector<std::pair<size_t, std::shared_ptr<ITransformer>>> transforms;
    };

#pragma region TransformPool
    // "Factory" class for transform chains -- vectors of ITransformer instances
    // Can get vector of unspecified transformers, and transformers mapped to a certain element of an arbitrary state
    // TODO: case when there is no U
    template <typename T, typename U>
    class TransformBuilder {
    public:
        std::function<void(T&, U)> func;
        std::vector<U> packedArgs;

        TransformBuilder(std::function<void(T&, U)> f, std::vector<U> vec)
            : func(f), packedArgs(vec) {}

        std::vector<std::shared_ptr<ITransformer>> GetTransformers() {
            auto v = std::vector<std::shared_ptr<ITransformer>>();

            for (auto &i : packedArgs) {
                auto t = ConstructTransformer<T, U>(func, std::decay_t<U>(i));
                v.push_back(t);
            }

            return v;
        }

        // Produce a vector of transformers that are to be applied on a variable at 'index' in the given state vector
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> MapTransformersToStateIndex(size_t index) {
            auto v = std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>();
            
            for (auto &i : packedArgs) {
                auto t = ConstructTransformer<T, U>(func, std::decay_t<U>(i));
                v.push_back(std::make_shared<std::pair<size_t, std::shared_ptr<ITransformer>>>(std::make_pair(index, t)));
            }

            return v;
        }
    };

#pragma endregion
    template <typename T>
    std::vector<std::shared_ptr<ITransformer>> GetTransformersForType() {
        auto v = std::vector<std::shared_ptr<ITransformer>>();
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