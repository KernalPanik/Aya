#pragma once

#include <vector>
#include <any>

void printVersion();

template <typename T>
std::vector<T> VecFlat(std::vector<std::vector<T>> vecs) {
    std::vector<T> result;
    for (auto &v : vecs) {
        result.insert(result.end(), v.begin(), v.end());
    }

    return result;
}

template <typename... Args, std::size_t... I>
std::tuple<Args...> ConstructTupleFromVec(const std::vector<std::any>& v, std::index_sequence<I...>) {
    return std::make_tuple(std::any_cast<Args>(v[I])...);
}

template <typename... Args>
std::tuple<Args...> Tuplify(std::vector<std::any> v) {
    if (v.size() != sizeof...(Args)) {
        throw std::invalid_argument("Vector size doesn't match tuple type count");
    }
    return ConstructTupleFromVec<Args...>(v, std::index_sequence_for<Args...>{});
}

std::vector<size_t> shuffle(const std::vector<size_t>& originalStorage, const size_t wantedSize);