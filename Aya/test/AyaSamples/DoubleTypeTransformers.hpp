#pragma once
#include <cmath>

#include "StandardMathTest.hpp"

#define PI 3.1415926

inline std::string doubleTypeToString(const std::any& value) {
    auto v = std::any_cast<double>(value);
    std::ostringstream os;
    os << std::setprecision(15) << v;
    return os.str();
}

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

inline void Tan(double &b) {
    b = tan(b * PI / 180);
}

inline void Atan(double &b) {
    b = atan(b) * 180 / PI;
}

inline void Asin(double &b) {
    if (b > 1 || b < -1) {
        throw std::domain_error("Invalid argument");
    }
    b = asin(b) * 180 / PI;
}

inline void Acos(double &b) {
    if (b > 1 || b < -1) {
        throw std::domain_error("Invalid argument");
    }
    b = acos(b) * 180 / PI;
}

inline void Sin2(double &b) {
    b = sin(b * PI / 180) * sin(b * PI / 180);
}

inline void Cos2(double &b) {
    b = cos(b * PI / 180) * cos(b * PI / 180);
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
    return fabs(x - y) < 1e-6 || (isnan(x) && isnan(y));
}

inline bool equalsWithMorePrecision(const double x, const double y) {
    return fabs(x - y) < 1e-12 || (isnan(x) && isnan(y));
}