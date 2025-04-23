#pragma once
#include <iostream>

#include "LinalgUtils/LinalgTransformers.hpp"
#include "AyaCore.h"

#include <Accelerate/Accelerate.h>

inline void GenerateMRsForMatrixMul(const std::function<double(double)> &testedFunction,
                                    const std::function<bool(double, double)> &comparer,
                                    const std::string &outputMRFile,
                                    size_t inputTransformerChainLength,
                                    size_t outputTransformerChainLength,
                                    size_t leftValueIndex,
                                    size_t rightValueIndex,
                                    const std::vector<std::vector<std::any> > &testedInputs,
                                    const std::vector<std::vector<std::any> > &validatorInputs) {
    std::array vec = {1.0, 0.0};
    std::array vec1 = {1.0, 0.0};
    for (auto &v: vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    Accelerate_Rotate90deg(vec);
    Accelerate_Rotate90deg(vec);
    Accelerate_Rotate90deg(vec);
    Accelerate_Rotate90deg(vec);
    for (auto &v: vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    std::cout << "Equals: " << vec2dEqual(vec, vec1) << std::endl;
}
