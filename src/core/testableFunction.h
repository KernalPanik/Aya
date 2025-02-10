#pragma once

#include <functional>

template <typename T, class... Types> 
class TestableFunction
{
public:
    TestableFunction(std::function<T(Types... args)> f) {
        this->func = f;
    }

    T Invoke(Types... args) {
        if (!std::is_void(T))
        {
            this->result = this->func(args...);
            return result;
        }
    }

private:
    std::function<T(Types... args)> func;
    T result;
    // Tuple Argument State -- values of all arguments at the given point TODO:
};