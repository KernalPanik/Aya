#pragma once

#include <iostream>
#include <functional>
#include <tuple>

template <typename T, class... Args> 
class TestableFunction
{
public:
    TestableFunction(std::function<T(Args&&... args)> f) {
        this->func = f;
    }

    std::tuple<T, Args...> Invoke(Args&&... args) {
        auto r = this->func(std::forward<Args>(args)...);
        // TODO: override make_tuple to make forwarded (rvalue) const values from rvalue references or references
        return std::make_tuple(r, std::forward<Args>(args)...);
    }

private:
    std::function<T(Args&&... args)> func;
};

template <class... Args> 
class TestableVoidFunction
{
public:
    TestableVoidFunction(std::function<void(Args... args)> f) {
        this->func = f;
    }

    void Invoke(Args... args) {
        std::cout << "Called a void function!" << std::endl;
        this->func(args...);
    }

private:
    std::function<void(Args... args)> func;
};