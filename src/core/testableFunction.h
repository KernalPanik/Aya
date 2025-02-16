#pragma once

#include <iostream>
#include <functional>
#include <tuple>
#include <optional>

template <typename T, class... Args> 
class TestableFunction
{
public:
    TestableFunction(std::function<T(Args&&... args)> f) {
        this->func = f;
    }

   std::tuple<T, Args... > Invoke(Args&&... args) {
        auto r = this->func(std::forward<Args>(args)...);
        auto x = std::tie(r, args...);
        return x;
   }

private:
    std::function<T(Args&&... args)> func;
};

template <class... Args> 
class TestableFunction<void, Args...>
{
public:
    TestableFunction(std::function<void(Args&&... args)> f) {
        this->func = f;
    }

   std::tuple<Args... > Invoke(Args&&... args) {
        this->func(std::forward<Args>(args)...);
        auto x = std::tie(args...);
        return x;
   }

private:
    std::function<void(Args&&... args)> func;
};