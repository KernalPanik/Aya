#include "StandardMathTest.hpp"
#include "PowTest.hpp"
#include "LinalgTest.hpp"
#include "TextConversionTests.hpp"
#include <list>

void RunFullTest() {
#pragma region TrigTest_Sine_Cosine
    GenerateMRsForDoubleDoubleArgFunc(SineSquared, equals,
        std::filesystem::current_path().generic_string() + "/Sine2Test.txt",
        1, 3,
        0, 1,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
    GenerateMRsForDoubleDoubleArgFunc(CosineSquared, equals,
        std::filesystem::current_path().generic_string() + "/Cosine2Test.txt",
        1, 3,
        0, 1,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
    GenerateMRsForDoubleDoubleArgFunc(SineSquared, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/Sine2TestHighPrecision.txt",
        1, 3,
        0, 1,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
    GenerateMRsForDoubleDoubleArgFunc(CosineSquared, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/Cosine2TestHighPrecision.txt",
        1, 3,
        0, 1,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
#pragma endregion
#pragma region TrigTest_Tan_Atan
    GenerateMRsForDoubleDoubleArgFunc(TanFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/TanTestHighPrecision.txt",
        1, 3,
        0, 1,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
    GenerateMRsForDoubleDoubleArgFunc(TanFunc, equals,
        std::filesystem::current_path().generic_string() + "/TanTest.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});

    GenerateMRsForDoubleDoubleArgFunc(AtanFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/AtanTestHighPrecision.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
    GenerateMRsForDoubleDoubleArgFunc(AtanFunc, equals,
        std::filesystem::current_path().generic_string() + "/AtanTest.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
#pragma endregion
#pragma region TrigTest_Asin_Acos
    // asin(sin(sin(x))) = sin(asin(sin(x)))
    GenerateMRsForDoubleDoubleArgFunc(AsinFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/AsinTestHighPrecision.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(AsinFunc, equals,
        std::filesystem::current_path().generic_string() + "/AsinTest.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});


    GenerateMRsForDoubleDoubleArgFunc(AcosFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/AcosTestHighPrecision.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(AcosFunc, equals,
        std::filesystem::current_path().generic_string() + "/AcosTest.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
#pragma endregion
#pragma region Pow()
    GenerateMRsForPow(Pow, equals, std::filesystem::current_path().generic_string() + "/PowerFuncTest.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 1, 10), Aya::GenerateDoublesAsAny(10, 1, 5)},
        {Aya::GenerateDoublesAsAny(10, 1, 10), Aya::GenerateDoublesAsAny(10, 1, 5)});
    GenerateMRsForPow(Pow, equalsWithMorePrecision, std::filesystem::current_path().generic_string() + "/PowerFuncTestHighPrecision.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 1, 10), Aya::GenerateDoublesAsAny(10, 1, 5)},
        {Aya::GenerateDoublesAsAny(10, 1, 10), Aya::GenerateDoublesAsAny(10, 1, 5)});
#pragma endregion

#pragma region log_exp_root
    GenerateMRsForDoubleDoubleArgFunc(ExpFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/ExpTestHighPrecision.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(ExpFunc, equals,
        std::filesystem::current_path().generic_string() + "/ExpTest.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});

    GenerateMRsForDoubleDoubleArgFunc(LogFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/LogTestHighPrecision.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(LogFunc, equals,
        std::filesystem::current_path().generic_string() + "/LogTest.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});

    GenerateMRsForDoubleDoubleArgFunc(RootFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/RootTestHighPrecision.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(RootFunc, equals,
        std::filesystem::current_path().generic_string() + "/RootTest.txt",
        1, 3,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
#pragma endregion

#pragma region linalg
    GenerateMRsForMatrixMul(Vec2, vec2dEqual,
        std::filesystem::current_path().generic_string() + "/MatrixMulTest.txt",
        5, 2,
        0, 0,
        {Aya::Generate2dVectorsAsAny(5, 0, 60)},
        {Aya::Generate2dVectorsAsAny(5, 0, 60)});

    GenerateMRsForMatrixMul(Vec2, vec2dEqualHighPrecision,
    std::filesystem::current_path().generic_string() + "/MatrixMulTestHighPrecision.txt",
    5, 2,
    0, 0,
    {Aya::Generate2dVectorsAsAny(5, 0, 60)},
    {Aya::Generate2dVectorsAsAny(5, 0, 60)});
#pragma endregion

#pragma region encodingTests
    EncodedString inputStr = EncodedString("森 ąžuolas звезда по имени солнце ");
    EncodedString validatorStr = EncodedString("meška ir kopūstas пёс - друг человека にゃん猫");
    GenerateMRsForTextEncoding(EncodeStringAsUtf8, CompareEncodedStrings,
                               std::filesystem::current_path().generic_string() + "/EncodingTest.txt",
                               1,
                               5,
                               0,
                               0,
                               {{inputStr}},
                               {{validatorStr}});
#pragma endregion
}

void RunManyRotationsTest(size_t rotationCount) {
    std::array<double, 2> vec = {1.0, 0.0};

    std::cout << "Rotating vector " << vec[0] << "; " << vec[1] << " by 15 degrees " << rotationCount << " times." << std::endl;

    for (auto i : vec) {
        std::cout << std::setprecision(15) << i << " ";
    }

    for (size_t i = 0; i < rotationCount; i++) {
        Accelerate_Rotate15deg(vec);
    }

    for (auto i : vec) {
        std::cout << std::setprecision(15) << i << " ";
    }
    std::cout << std::endl;
}

void RunSinAsinTest(size_t iterationCount) {
    std::cout << "test";
    std::cout << "Applying Sin " << iterationCount << " times, followed by asin for the same amount of times" << std::endl;
    double val_deg = 45.0;
    double val_rad = val_deg * M_PI / 180.0;

    std::cout << "start value (rad) " << val_rad << std::endl;
    std::cout << "start value (deg) " << val_deg << std::endl;
    for (size_t i = 0; i < iterationCount; i++) {
        val_rad = sin(val_rad);
    }
    for (size_t i = 0; i < iterationCount; i++) {
        val_rad = asin(val_rad);
    }
    val_deg = val_rad * 180/M_PI;
    std::cout << std::setprecision(15) << "end value (rad): " << val_rad << std::endl;
    std::cout << std::setprecision(15) << "end value (deg): " << val_deg << std::endl;
}

inline double Tax(double income) {
    if (income < 0.0) {
        return 0.0;
    }

    if (income > 60000) {
        return income / 3;
    }

    return income / 4;
}

int main() {
    GenerateMRsForDoubleDoubleArgFunc(mul2, equals,
            std::filesystem::current_path().generic_string() + "/Mul2Test.txt",
            1, 1,
            0, 1,
            {{1.0, 4.0, 9.0}},
            {{10.0, 20.0, 30.0}});

    std::vector<std::any> validationVals;
    for (size_t i = 0; i < 100; i++) {
        validationVals.push_back(i * 1000.0);
    }

    GenerateMRsForDoubleDoubleArgFunc(Tax, equalsWithMorePrecision,
    std::filesystem::current_path().generic_string() + "/TaxTest.txt",
    1, 3,
    0, 1,
    {{26000.0, 150.0, 90000.0, 90001.0}},
    {validationVals});

    //RunFullTest();
    //RunManyRotationsTest(24*15*1000000);
    //RunSinAsinTest(24000);
}