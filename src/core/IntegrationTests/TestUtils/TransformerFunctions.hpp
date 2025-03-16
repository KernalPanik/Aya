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

// TODO: Noop function must be implicitly applied to every ITransformer pool.
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
        vec.pop_back();
    }
}

inline void VecNoop(std::vector<int>& vec) {}
inline void VecNoop1(std::vector<int>& vec, const int val) {}
#pragma endregion