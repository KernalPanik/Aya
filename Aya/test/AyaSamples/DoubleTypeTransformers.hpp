#pragma once
#include <cmath>

#define PI 3.1415926

inline void Add(double &b, double val) {
    b += val;
}

inline void Mul(double &b, double val) {
    b *= val;
}

inline void Sub(double &b, double val) {
    b -= val;
}

inline void Div(double &b, double val) {
    b /= val;
}

inline void Sin(double &b) {
    b = sin(b * PI / 180);
}

inline void Cos(double &b) {
    b = cos(b * PI / 180);
}

inline void SinDivCos(double &b) {
    auto c = cos(b * PI / 180);
    auto s = sin(b * PI / 180);
    b = s / c;
}

inline void CosDivSin(double &b) {
    auto c = cos(b * PI / 180);
    auto s = sin(b * PI / 180);
    b = c / s;
}

inline void Sqrt(double &b) {
    b = sqrt(b);
}

inline void Square(double &b) {
    b = b * b;
}

inline void Noop(double &b, double val) {}

inline bool equals(const double x, const double y) {
    return fabs(x - y) < 1e-6;
}

inline bool equalsWithMorePrecision(const double x, const double y) {
    return fabs(x - y) < 1e-13;
}