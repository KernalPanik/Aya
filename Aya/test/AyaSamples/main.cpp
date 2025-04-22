#include "DoubleDoubleTest.hpp"
#include "TripleDoubleTest.hpp"

int main() {
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
        1, 1,
        0, 1,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
    GenerateMRsForDoubleDoubleArgFunc(TanFunc, equals,
        std::filesystem::current_path().generic_string() + "/TanTest.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});

    GenerateMRsForDoubleDoubleArgFunc(AtanFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/AtanTestHighPrecision.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
    GenerateMRsForDoubleDoubleArgFunc(AtanFunc, equals,
        std::filesystem::current_path().generic_string() + "/AtanTest.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 30, 90)},
        {Aya::GenerateDoublesAsAny(10, 30, 90)});
#pragma endregion
#pragma region TrigTest_Asin_Acos
    // asin(sin(sin(x))) = sin(asin(sin(x)))
    GenerateMRsForDoubleDoubleArgFunc(AsinFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/AsinTestHighPrecision.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(AsinFunc, equals,
        std::filesystem::current_path().generic_string() + "/AsinTest.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});


    GenerateMRsForDoubleDoubleArgFunc(AcosFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/AcosTestHighPrecision.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(AcosFunc, equals,
        std::filesystem::current_path().generic_string() + "/AcosTest.txt",
        1, 1,
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
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(ExpFunc, equals,
        std::filesystem::current_path().generic_string() + "/ExpTest.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});

    GenerateMRsForDoubleDoubleArgFunc(LogFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/LogTestHighPrecision.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(LogFunc, equals,
        std::filesystem::current_path().generic_string() + "/LogTest.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});

    GenerateMRsForDoubleDoubleArgFunc(RootFunc, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/RootTestHighPrecision.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
    GenerateMRsForDoubleDoubleArgFunc(RootFunc, equals,
        std::filesystem::current_path().generic_string() + "/RootTest.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(10, 0, 0)},
        {Aya::GenerateDoublesAsAny(10, 0, 0)});
#pragma endregion
}