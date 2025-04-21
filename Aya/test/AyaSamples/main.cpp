#include "DoubleDoubleTest.hpp"
#include "TripleDoubleTest.hpp"

int main() {
#pragma region TrigTest_Sine_Cosine
    GenerateMRsForDoubleDoubleArgFunc(SineSquared, equals,
        std::filesystem::current_path().generic_string() + "/SineTest.txt",
        1, 3,
        0, 1,
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)},
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)});
    GenerateMRsForDoubleDoubleArgFunc(CosineSquared, equals,
        std::filesystem::current_path().generic_string() + "/CosineTest.txt",
        1, 3,
        0, 1,
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)},
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)});
    GenerateMRsForDoubleDoubleArgFunc(SineSquared, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/SineTestHighPrecision.txt",
        1, 3,
        0, 1,
        {Aya::GenerateDoublesAsAny(4, 1, 10)},
        {Aya::GenerateDoublesAsAny(4, 1, 10)});
    GenerateMRsForDoubleDoubleArgFunc(CosineSquared, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/CosineTestHighPrecision.txt",
        1, 3,
        0, 1,
        {Aya::GenerateDoublesAsAny(4, 1, 10)},
        {Aya::GenerateDoublesAsAny(4, 1, 10)});
#pragma endregion
#pragma region TrigTest_Tan_Atan
    GenerateMRsForDoubleDoubleArgFunc(Tan, equals,
        std::filesystem::current_path().generic_string() + "/TanTestHighPrecision.txt",
        1, 1,
        0, 1,
        {Aya::GenerateDoublesAsAny(4, 1, 10)},
        {Aya::GenerateDoublesAsAny(4, 1, 10)});
#pragma endregion
#pragma region Pow()
    GenerateMRsForPow(Pow, equals, std::filesystem::current_path().generic_string() + "/PowerFuncTest.txt",
        1, 1,
        0, 0,
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10), Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)},
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10), Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)});
    GenerateMRsForPow(Pow, equalsWithMorePrecision, std::filesystem::current_path().generic_string() + "/PowerFuncTestHighPrecision.txt",
        1, 1,
        0, 0,
        {Aya::GenerateDoublesAsAny(4, 1, 10), Aya::GenerateDoublesAsAny(4, 1, 10)},
        {Aya::GenerateDoublesAsAny(5, 1, 10), Aya::GenerateDoublesAsAny(5, 1, 10)});
#pragma endregion
}