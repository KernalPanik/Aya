#pragma once

#pragma region Transformers For Pow()
inline void Add(double& b, double val) {
    b += val;
}

inline void Mul(double& b, double val) {
    b *= val;
}

inline void Sub(double& b, double val) {
    b -= val;
}

inline void Div(double& b, double val) {
    b /= val;
}

inline void Noop(double& b, double val) {}

#pragma endregion

#pragma region Transformers For VecSize()
inline void push(std::vector<int>& vec, const int val, const int repeat) {
    for (int i = 0; i < repeat; i++) {
        vec.push_back(val);
    }
}

inline void pop(std::vector<int>& vec, const int repeat) {
    for (int i = 0; i < repeat; i++) {
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
#pragma endregion