#pragma once

#include <vector>
#include <random>

namespace Aya {
    /*
     * n -- number of doubles to generate
     * rangeMin -- minimal integer value of the generated number
     * rangeMax -- max integer value of the generated number
     *
     *  (10, 100) will return random double values in range 10.<rand> -> 100.<rand>
     */
    inline std::vector<std::any> GenerateDoublesAsAny(const size_t n, const int rangeMin, const int rangeMax) {
        std::vector<std::any> result;
        result.reserve(n);
        static std::mt19937_64 generator(std::random_device{}());
        std::uniform_int_distribution<> beforeDotGen(rangeMin, rangeMax);
        for (size_t i = 0; i < n; i++) {
            const auto afterDot = std::generate_canonical<double, 128>(generator);
            const auto beforeDot = static_cast<double>(beforeDotGen(generator));
            result.emplace_back(beforeDot + afterDot);
        }

        return result;
    }

    inline std::vector<double> GenerateNDimVectorAsAny(const size_t dim, const int rangeMin, const int rangeMax) {
        std::vector<double> result;
        auto v = GenerateDoublesAsAny(dim, rangeMin, rangeMax);
        result.reserve(dim);
        for (size_t i = 0; i < dim; i++) {
            result.push_back(std::any_cast<double>(v[i]));
        }
        return result;
    }

    inline std::vector<std::any> Generate2dVectorsAsAny(const size_t n, const int rangeMin, const int rangeMax) {
        std::vector<std::any> result;

        for (size_t i = 0; i < n; i++) {
            auto v = GenerateNDimVectorAsAny(2, rangeMin, rangeMax);
            std::array<double, 2> arr;
            arr[0] = v[0];
            arr[1] = v[1];

            result.emplace_back(arr);
        }

        return result;
    }
}
