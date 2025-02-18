#pragma once

#include "transformer.h"

#include <vector>
#include <iostream>

template<typename... Args>
void func(const Args&... args) {
    (std::cout << ... << args ) << std::endl;
}

 //func_vec.push_back([]() { func(1, "test"); });
 //   func_vec.push_back([]() { func(1.4, "testas", "22.5"); });

struct MetamorphicRelation
{
    // such vector is capable of storing any vararg function, defined as func template
    std::vector<std::function<void()>> func_vec;
};
