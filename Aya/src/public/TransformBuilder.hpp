#pragma once

#include "CoreUtilities.hpp"

#include "transformer.hpp"

namespace Aya {
    template<typename T, typename... Args, typename Callable>
    std::shared_ptr<ITransformer> ConstructTransformer(Callable &&f, std::string functionName, Args &&... args) {
        return std::make_shared<Transformer<T, Args...>>(functionName, std::forward<Callable>(f),
                                                          std::forward<Args>(args)...);
    }

    template<typename T, typename Callable>
    std::shared_ptr<ITransformer> ConstructTransformer(Callable &&f, std::string functionName) {
        return std::make_shared<NoArgumentTransformer<T>>(functionName, std::forward<Callable>(f));
    }

    template<typename T, typename... Args>
    std::shared_ptr<ITransformer> ConstructTransformer(std::function<void(T &, Args...)> f, std::string functionName,
                                                       std::tuple<Args...> args) {
        return std::make_shared<Transformer<T, Args...>>(functionName, f, args);
    }

    template<typename T, typename... Args>
    class TransformBuilder {
    public:
        TransformBuilder() = default;

        // Single function, multiple argument variants
        std::vector<std::shared_ptr<ITransformer>> GetTransformers(std::function<void(T &, Args...)> func,
                                                                    const std::string &functionName,
                                                                    std::vector<std::tuple<Args...>> params) {
            return GetTransformersInternal(functionName, func, params);
        }

        // Functions without arguments
        std::vector<std::shared_ptr<ITransformer>> GetTransformers(
                std::vector<std::function<void(T &, Args...)>> func,
                const std::vector<std::string> &functionNames) {
            std::vector<std::shared_ptr<ITransformer>> v;

            for (size_t i = 0; i < func.size(); ++i) {
                auto transformers = GetTransformersInternal(functionNames[i], func[i]);
                v.insert(v.end(), transformers.begin(), transformers.end());
            }
            return v;
        }

        std::vector<std::shared_ptr<ITransformer>> GetTransformers(std::function<void(T &, Args...)> func,
                                                                    const std::string &functionName) {
            return GetTransformersInternal(functionName, func);
        }

        std::vector<std::shared_ptr<ITransformer>> GetTransformers(
                std::vector<std::function<void(T &, Args...)>> func,
                const std::vector<std::string> &functionNames,
                std::vector<std::vector<std::tuple<Args...>>> params) {
            std::vector<std::shared_ptr<ITransformer>> v;
            if (func.size() != params.size()) {
                throw std::invalid_argument("Vector of functions must match the vector of arguments.");
            }

            for (size_t i = 0; i < params.size(); ++i) {
                auto transformers = GetTransformersInternal(functionNames[i], func[i], params[i]);
                v.insert(v.end(), transformers.begin(), transformers.end());
            }
            return v;
        }

    private:
        std::vector<std::shared_ptr<ITransformer>> GetTransformersInternal(
            std::string functionName, std::function<void(T &, Args...)> func,
            std::vector<std::tuple<Args...>> params) {
            std::vector<std::shared_ptr<ITransformer>> v;
            for (size_t i = 0; i < params.size(); ++i) {
                auto t = ConstructTransformer<T, Args...>(func, functionName, params[i]);
                v.push_back(t);
            }

            return v;
        }

        std::vector<std::shared_ptr<ITransformer>> GetTransformersInternal(
            std::string functionName, std::function<void(T &, Args...)> func) {
            std::vector<std::shared_ptr<ITransformer>> v;
            auto t = ConstructTransformer<T, Args...>(func, functionName);
            v.push_back(t);

            return v;
        }
    };
}
