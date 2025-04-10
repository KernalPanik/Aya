#include <iostream>

#include "SineTest.hpp"

int main() {
    std::cout << "Hello, World!" << std::endl;

//    GenerateMRsForSine();

    for (size_t i = 0; i < 26; i++) {
        std::cout << "sin(" << 15 * i << ") = " << sine(15 * i) << std::endl;
    }

    return 0;
}