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

#define PI 3.14159265
inline void Sin(double& b, double val) {
  //b = sin(b) * sin(b);
  b = sin(b*PI/180) * sin(b*PI/180);
}

inline void Cos(double& b, double val) {
  //b = cos(b) * cos(b);
  b = cos(b*PI/180) * cos(b*PI/180);
}

inline void Sqrt(double& b, double val) {
  b = sqrt(b);
}

inline void Square(double& b, double val) {
  b = b * b;
}

inline void Noop(double& b, double val) {}