#pragma once

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