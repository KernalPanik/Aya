#pragma once

#include <tuple>
#include <string>

/*
template <class... Args>
class FunctionState
{
public:
    // For now, expected calling scenario is to pass the product of make_tuple() to this ctor
    FunctionState(Args&&... args) {
        this->value = std::make_tuple(std::forward<Args>(args)...);
    }

    FunctionState(Args&&... args) {
        this->value = std::make_tuple(std::forward<Args>(args)...);
    }
    bool equals(std::tuple<Args&&...> v) {
        return false;
    }

    std::string ToString() {
        return std::string("Sanity");
    }

private:
    std::tuple<Args&&...> value;
};
*/