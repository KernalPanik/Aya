#pragma once

#include "transformer.hpp"

// TODO: case when there is no U
// Make a specialization for cases where U is void. (make U void as default then), and override every function.
// At this point, perhaps simply create two classes matching the same interface?
namespace Aya {
    template<typename T, typename... Args, typename Callable>
        std::shared_ptr<ITransformer> ConstructTransformer(Callable&& f, Args&&... args) {
        return std::make_shared<Transformer<T, Args...>>(std::forward<Callable>(f), std::forward<Args>(args)...);
    }

    // T is the modifiable (base) type, U is the modifier type
    template <typename T, typename U>
    class TransformBuilder {
    public:
        TransformBuilder(std::function<void(T&, U)> f, std::vector<U> vec)
            : m_TransformerFunction(std::make_unique<std::function<void(T&, U)>>(f)), m_PackedArgs(std::make_unique<std::vector<U>>(vec)) {}

        TransformBuilder()
           : m_TransformerFunction(nullptr), m_PackedArgs(nullptr) {}

        std::vector<std::shared_ptr<ITransformer>> GetTransformers(const std::vector<std::function<void(T&, U)>>& transformerFunctions,
            const std::vector<std::vector<U>>& params) {
            std::vector<std::shared_ptr<ITransformer>> v;

            for (size_t i = 0; i < params.size(); ++i) {
                auto transformers = GetTransformersInternal(transformerFunctions[i], params[i]);
                v.insert(v.end(), transformers.begin(), transformers.end());
            }

            return v;
        }

        // Produce a vector of transformers that are to be applied on a variable at 'index' in the given state vector
        //TODO: make this function return a map instead of vector of pairs
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> MapTransformersToStateIndex(size_t index) {
            auto v = std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>();
            auto transformers = GetTransformersInternal(*m_TransformerFunction, *m_PackedArgs);

            for (auto &i : transformers) {
                v.push_back(std::make_shared<std::pair<size_t, std::shared_ptr<ITransformer>>>(std::make_pair(index, std::move(i))));
            }

            return v;
        }

    private:
        std::unique_ptr<std::function<void(T&, U)>> m_TransformerFunction;
        std::unique_ptr<std::vector<U>> m_PackedArgs;

        std::vector<std::shared_ptr<ITransformer>> GetTransformersInternal(std::function<void(T&, U)> func, std::vector<U> args) {
            auto v = std::vector<std::shared_ptr<ITransformer>>();

            for (auto &i : args) {
                auto t = ConstructTransformer<T, U>(func, std::decay_t<U>(i));
                v.push_back(t);
            }

            return v;
        }
    };

    // If Transformer functions can be properly done with a single arg of known type, refer to TransformBuilder for now.
    template <typename T, typename... Args>
    class TransformBuilderVararg {
    public:
        explicit TransformBuilderVararg(std::function<void(T&, Args...)> func, Args... args)
            : m_TransformerFunction(func) {}

        TransformBuilderVararg() : m_TransformerFunction(nullptr) {}

        std::vector<std::shared_ptr<ITransformer>> GetTransformers(std::vector<std::tuple<Args...>> params) {
            return GetTransformersInternal(params);
        }

        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> MapTransformersToStateIndex(std::vector<std::tuple<Args...>> params, size_t index) {
            auto v = std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>();
            auto transformers = GetTransformersInternal(params);

            for (auto &i : transformers) {
                v.push_back(std::make_shared<std::pair<size_t, std::shared_ptr<ITransformer>>>(std::make_pair(index, std::move(i))));
            }
            return v;
        }

    private:
        std::unique_ptr<std::function<void(T&, Args...)>> m_TransformerFunction;

        std::vector<std::shared_ptr<ITransformer>> GetTransformersInternal(std::vector<std::tuple<Args...>> params) {
            std::vector<std::shared_ptr<ITransformer>> v;
            for (size_t i = 0; i < params.size(); ++i) {
                auto t = ConstructTransformer<T, Args...>(m_TransformerFunction, params[i]);
                v.push_back(t);
            }

            return v;
        }
    };
}