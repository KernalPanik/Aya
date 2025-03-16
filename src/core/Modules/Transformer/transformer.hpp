#pragma once

#include <functional>
#include <any>

namespace Aya {
    class ITransformer {
    public:
        virtual ~ITransformer() = default;
        virtual void Apply(void* data) = 0;
        // NOTE: not applicable in C# context
        virtual void Apply(std::any& data) = 0;
        virtual size_t GetArgCount() = 0;
    };

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

        size_t GetArgCount() override {
            return sizeof...(Args);
        }

    private:
        std::function<void(T&, Args...)> func;
        std::tuple<Args...> args;

        template<std::size_t... I>
        void ApplyImpl(std::index_sequence<I...>, T& baseValue) {
            func(baseValue, std::forward<Args>(std::get<I>(args))...);
        }
    };
}