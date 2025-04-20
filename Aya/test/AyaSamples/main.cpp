#include "TrigTest.hpp"
#include "PowTest.hpp"

int main() {
#pragma region TrigTest
    GenerateMRsForSineOrCosLowPrecision(sineSquared, equals,
        std::filesystem::current_path().generic_string() + "/SineTest.txt",
        1, 3,
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)},
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)});
    GenerateMRsForSineOrCosLowPrecision(cosineSquared, equals,
        std::filesystem::current_path().generic_string() + "/CosineTest.txt",
        1, 3,
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)},
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)});
    GenerateMRsForSineOrCosLowPrecision(sineSquared, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/SineTestHighPrecision.txt",
        1, 3,
        {Aya::GenerateDoublesAsAny(4, 1, 10)},
        {Aya::GenerateDoublesAsAny(4, 1, 10)});
    GenerateMRsForSineOrCosLowPrecision(cosineSquared, equalsWithMorePrecision,
        std::filesystem::current_path().generic_string() + "/CosineTestHighPrecision.txt",
        1, 3,
        {Aya::GenerateDoublesAsAny(4, 1, 10)},
        {Aya::GenerateDoublesAsAny(4, 1, 10)});
#pragma endregion
#pragma region Pow()
    GenerateMRsForPow(Pow, equals, std::filesystem::current_path().generic_string() + "/PowerFuncTest.txt",
        1, 1,
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10), Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)},
        {Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10), Aya::GenerateNoDecimalDoublesAsAny(4, 1, 10)});
    GenerateMRsForPow(Pow, equalsWithMorePrecision, std::filesystem::current_path().generic_string() + "/PowerFuncTestHighPrecision.txt",
        1, 1,
        {Aya::GenerateDoublesAsAny(4, 1, 10), Aya::GenerateDoublesAsAny(4, 1, 10)},
        {Aya::GenerateDoublesAsAny(5, 1, 10), Aya::GenerateDoublesAsAny(5, 1, 10)});
#pragma endregion
    return 0;
}