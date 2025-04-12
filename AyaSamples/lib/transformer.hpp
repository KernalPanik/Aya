#pragma once

#include <functional>
#include <any>
#include "tuple-utils.h"

namespace Aya {
    class ITransformer {
    public:
        virtual ~ITransformer() = default;
        virtual void Apply(void* data) = 0;
        // NOTE: might not be applicable in C# context
        virtual void Apply(std::any& data) = 0;
        virtual size_t GetArgCount() = 0;
        virtual void SetRepeat(size_t val) = 0;
        [[nodiscard]]
        virtual size_t GetRepeat() = 0;
        [[nodiscard]]
        virtual std::string ToString(const char* targetName, size_t inputIndex) = 0;
        virtual void OverrideArgNames(std::vector<std::string> newNames) = 0;
        virtual void OverrideArgs(std::vector<void*> newArgs) = 0;
        virtual void OverrideArgs(const std::vector<std::any>& newArgs) = 0;
        virtual std::shared_ptr<ITransformer> Clone() = 0;
    };

    // TODO: no arg transformer
    template<typename T, class... Args>
    class Transformer final : public ITransformer {
    public:
        explicit Transformer(const std::string& functionName, std::function<void(T&, Args...)> f, Args&&... args)
            : m_Func(f), m_FunctionName(functionName), m_Args(std::make_tuple(std::forward<Args>(args)...)), m_ArgNames(std::vector<std::string>()) {}

        Transformer(const std::string& functionName, std::function<void(T&, Args...)> f, std::tuple<Args...> args)
            : m_Func(f), m_FunctionName(functionName), m_Args(args), m_ArgNames(std::vector<std::string>()) {}

        void Apply(void* data) override {
            if (data == nullptr) {
                throw std::invalid_argument("Cannot transform base that is null.");
            }

            auto baseValue = *static_cast<T*>(data);
            for (size_t i = 0; i < m_Repeat; i++) {
                ApplyImpl(std::index_sequence_for<Args...>{}, baseValue);
            }
            memcpy(data, &baseValue, sizeof(baseValue));
        }

        void Apply(std::any& data) override {
            auto baseValue = std::any_cast<T>(data);
            for (size_t i = 0; i < m_Repeat; i++) {
                ApplyImpl(std::index_sequence_for<Args...>{}, baseValue);
            }
            data = baseValue;
        }

        size_t GetArgCount() override {
            return sizeof...(Args);
        }

        void SetRepeat(const size_t val) override {
            m_Repeat = val;
        }

        [[nodiscard]]
        size_t GetRepeat() override {
            return m_Repeat;
        }

        [[nodiscard]]
        std::string ToString(const char* targetName, const size_t inputIndex) override {
            std::stringstream ss;
            ss << m_FunctionName << "( " + std::string(targetName) +  "[" << std::to_string(inputIndex) << "], ";
            if constexpr (sizeof...(Args) > 0) {
                if (!m_ArgNames.empty()) {
                    for (size_t i = 0; i < m_ArgNames.size(); i++) {
                        ss << m_ArgNames[i];
                        ss << ", ";
                    }
                }
                else {
                    ss << TupleToString(m_Args);
                }
            }
            else {
                ss << "";
            }
            ss << ")";

            return ss.str();
        }

        void OverrideArgNames(const std::vector<std::string> newNames) override {
            m_ArgNames = newNames;
        }

        // C# i guess
        void OverrideArgs(std::vector<void*> newArgs) override {}

        void OverrideArgs(const std::vector<std::any>& newArgs) override {
            if (TupleVec(m_Args).size() != newArgs.size()) {
                throw std::invalid_argument("Invalid number of arguments to override.");
            }

            auto t = Tuplify<Args...>(newArgs);
            m_Args = t;
        }

        std::shared_ptr<ITransformer> Clone() override {
            auto a =  std::make_shared<Transformer<T, Args...>>(m_FunctionName, m_Func, m_Args);
            a->OverrideArgNames(m_ArgNames);
            a->SetRepeat(m_Repeat);
            return a;
        }

    private:
        std::function<void(T&, Args...)> m_Func;
        std::string m_FunctionName;
        std::tuple<Args...> m_Args;
        std::vector<std::string> m_ArgNames;

        size_t m_Repeat = 1;
        template<std::size_t... I>
        void ApplyImpl(std::index_sequence<I...>, T& baseValue) {
            std::tuple<Args...> vargs = std::forward_as_tuple(std::get<I>(m_Args)...);
            auto tup = std::make_tuple(baseValue);
            auto tups = std::tuple_cat(tup, vargs);
            std::apply(m_Func, tups);
            baseValue = static_cast<T>(std::get<0>(tups)); // BaseValue is always combined first with Args, so getting element 0 will yield what we want.
        }
    };
}
