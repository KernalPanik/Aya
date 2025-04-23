#pragma once

#include <iostream>
#include <array>

static bool vec2dEqual(const std::array<double, 2> v1, const std::array<double, 2> v2) {
    for (int i = 0; i < 2; i++) {
        auto diff = fabs(v1[i] - v2[i]);
        if (diff > 1e-6) {
            return false;
        }
    }

    return true;
}

static bool vec2dEqualHighPrecision(const std::array<double, 2> v1, const std::array<double, 2> v2) {
    for (int i = 0; i < 2; i++) {
        auto diff = fabs(v1[i] - v2[i]);
        if (diff > 1e-12) {
            return false;
        }
    }

    return true;
}


#pragma region Accelerate
#if __APPLE__
// Accelerate is macOS-only feature
#include <Accelerate/Accelerate.h>
#endif

static void Accelerate_MulMatrixBy2dVec(const std::array<double, 4> matrix, std::array<double, 2> &vec) {
#if __APPLE__
    double alpha = 1.0;
    double beta = 0.0;
    cblas_dgemv(
        CblasRowMajor, CblasNoTrans,
        2, 2, alpha, matrix.data(), 2, vec.data(), 1, beta, vec.data(), 1
    );
#else
    std::cout << "Accelerate API is not accessible on non Apple Systems. " << std::endl;
#endif
}

static std::array<double, 4> Get2dRotationMatrix(const double theta) {
    double thetaRadian = theta * M_PI / 180.0;
    const std::array result = {
        cos(thetaRadian), sin(thetaRadian) * -1,
        sin(thetaRadian), cos(thetaRadian)
    };
    return result;
}

inline void Accelerate_RotateNeg15deg(std::array<double, 2> &vec) {
    const auto rot = Get2dRotationMatrix(-15);
    Accelerate_MulMatrixBy2dVec(rot, vec);
}

inline void Accelerate_RotateNeg30deg(std::array<double, 2> &vec) {
    const auto rot = Get2dRotationMatrix(-30);
    Accelerate_MulMatrixBy2dVec(rot, vec);
}

inline void Accelerate_Rotate15deg(std::array<double, 2> &vec) {
    const auto rot = Get2dRotationMatrix(15);
    Accelerate_MulMatrixBy2dVec(rot, vec);
}

inline void Accelerate_Rotate30deg(std::array<double, 2> &vec) {
    const auto rot = Get2dRotationMatrix(30);
    Accelerate_MulMatrixBy2dVec(rot, vec);
}

inline void Accelerate_Rotate60deg(std::array<double, 2> &vec) {
    const auto rot = Get2dRotationMatrix(60);
    Accelerate_MulMatrixBy2dVec(rot, vec);
}

inline void Accelerate_Rotate90deg(std::array<double, 2> &vec) {
    const auto rot = Get2dRotationMatrix(90);
    Accelerate_MulMatrixBy2dVec(rot, vec);
}

inline void Accelerate_Rotate120deg(std::array<double, 2> &vec) {
    const auto rot = Get2dRotationMatrix(120);
    Accelerate_MulMatrixBy2dVec(rot, vec);
}

inline void Accelerate_Rotate150deg(std::array<double, 2> &vec) {
    const auto rot = Get2dRotationMatrix(150);
    Accelerate_MulMatrixBy2dVec(rot, vec);
}

#pragma endregion