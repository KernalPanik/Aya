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

    template<typename T, typename... Args>
    std::shared_ptr<ITransformer> ConstructTransformer(std::function<void(T&, Args...)> f, std::tuple<Args...> args) {
        return std::make_shared<Transformer<T, Args...>>(f, args);
    }

    // If Transformer functions can be properly done with a single arg of known type, refer to TransformBuilder for now.
    template <typename T, typename... Args>
    class TransformBuilder {
    public:
        TransformBuilder() = default;

        std::vector<std::shared_ptr<ITransformer>> GetTransformers(std::function<void(T&, Args...)> func, std::vector<std::tuple<Args...>> params) {
            return GetTransformersInternal(func, params);
        }

        std::vector<std::shared_ptr<ITransformer>> GetTransformers(std::vector<std::function<void(T&, Args...)>> func,
                std::vector<std::vector<std::tuple<Args...>>> params) {
            std::vector<std::shared_ptr<ITransformer>> v;
            if (func.size() != params.size()) {
                throw std::invalid_argument("Vector of functions must match the vector of arguments.");
            }

            for (size_t i = 0; i < params.size(); ++i) {
                auto transformers = GetTransformersInternal(func[i], params[i]);
                v.insert(v.end(), transformers.begin(), transformers.end());
            }
            return v;
        }

        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> MapTransformersToStateIndex(std::function<void(T&, Args...)> func,
                std::vector<std::tuple<Args...>> params, size_t index) {
            auto v = std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>>();
            auto transformers = GetTransformersInternal(func, params);

            for (auto &i : transformers) {
                v.push_back(std::make_shared<std::pair<size_t, std::shared_ptr<ITransformer>>>(std::make_pair(index, std::move(i))));
            }
            return v;
        }

    private:
        std::vector<std::shared_ptr<ITransformer>> GetTransformersInternal(std::function<void(T&, Args...)> func,
                std::vector<std::tuple<Args...>> params) {
            std::vector<std::shared_ptr<ITransformer>> v;
            for (size_t i = 0; i < params.size(); ++i) {
                auto t = ConstructTransformer<T, Args...>(func, params[i]);
                v.push_back(t);
            }

            return v;
        }
    };
}