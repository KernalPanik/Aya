#pragma once

#include <functional>
#include <any>
#include <src/common/tuple-utils.h>

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
        virtual std::string ToString() = 0;
    };

    template<typename T, class... Args>
    class Transformer final : public ITransformer {
    public:
        explicit Transformer(std::function<void(T&, Args...)> f, Args&&... args)
            : func(f), args(std::make_tuple(std::forward<Args>(args)...)) {}

        Transformer(std::function<void(T&, Args...)> f, std::tuple<Args...> args)
            : func(f), args(args) {}

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
        std::string ToString() override {
            //std::stringstream ss;
            //ss << TupleToString(args);
            if constexpr (sizeof...(Args) > 0) {
                return TupleToString(args);
            }
            else {
                return std::string("");
            }
        }

    private:
        std::function<void(T&, Args...)> func;
        std::tuple<Args...> args;

        size_t m_Repeat = 1;

        template<std::size_t... I>
        void ApplyImpl(std::index_sequence<I...>, T& baseValue) {
            func(baseValue, std::forward<Args>(std::get<I>(args))...);
        }
    };
}
