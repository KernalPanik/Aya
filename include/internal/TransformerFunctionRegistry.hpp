#pragma once
#include <vector>
#include <string>
#include <tuple>
#include <functional>
#include <memory>
#include "Transformer.hpp"

namespace Aya {
    /*
        Collection of functions, their arguments and extra metadata.
    */
    template<typename T, typename... Args>
    class TransformerFunctionRegistry {
    public:
        using FuncType = std::function<void(T&, Args...)>;
        using ArgTuple = std::tuple<Args...>;
        
        struct TransformerFunction {
            std::string name;
            FuncType func;
            std::vector<ArgTuple> argPool;
        };

        void RegisterFunction(const std::string& name, FuncType func, const std::vector<ArgTuple>& argPool) {
            entries.push_back({name, func, argPool});
        }

        // Getters for use in TransformBuilder
        std::vector<FuncType> GetFuncs() const {
            std::vector<FuncType> out;
            for (const auto& e : entries) out.push_back(e.func);
            return out;
        }
        std::vector<std::string> GetNames() const {
            std::vector<std::string> out;
            for (const auto& e : entries) out.push_back(e.name);
            return out;
        }
        std::vector<std::vector<ArgTuple>> GetArgPools() const {
            std::vector<std::vector<ArgTuple>> out;
            for (const auto& e : entries) out.push_back(e.argPool);
            return out;
        }

        size_t size() const { return entries.size(); }

    private:
        std::vector<TransformerFunction> entries;
    };
}