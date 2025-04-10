#pragma once
#include <cmath>

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

inline void Sin(double& b) {
  b = sin(b);
}

inline void Cos(double& b) {
  b = cos(b);
}

inline void Sqrt(double& b) {
  b = sqrt(b);
}

inline void Square(double& b) {
  b *= b;
}

inline void Noop(double& b, double val) {}