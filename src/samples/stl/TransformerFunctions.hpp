#pragma once

#include <vector>

inline void pushNTimes(std::vector<int>& vec, const int val, const int N) {
    for (int i = 0; i < N; i++) {
        vec.push_back(val);
    }
}

inline void popNTimes(std::vector<int>& vec, const int N) {
    for (int i = 0; i < N; i++) {
        if (!vec.empty()) // pop on empty vector is UB
            vec.pop_back();
    }
}

inline void VecAdd(size_t& b, size_t val) {
    b += val;
}

inline void VecSub(size_t& b, size_t val) {
    b -= val;
}

inline void VecNoop(std::vector<int>& vec) {}
