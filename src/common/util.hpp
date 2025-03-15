#pragma once

#include <vector>

void printVersion();

template <typename T>
std::vector<T> VecFlat(std::vector<std::vector<T>> vecs) {
    std::vector<T> result;
    for (auto &v : vecs) {
        result.insert(result.end(), v.begin(), v.end());
    }

    return result;
}

std::vector<size_t> shuffle(const std::vector<size_t>& originalStorage, const size_t wantedSize);