#include "TrigTest.hpp"
#include "PowTest.hpp"

int main() {
    GenerateMRsForSineOrCosLowPrecision(sineSquared, std::filesystem::current_path().generic_string() + "/SineTest.txt", 1, 3);
    //GenerateMRsForSineOrCosHighPrecision(sineSquared, std::filesystem::current_path().generic_string() + "/SineTestHighPrecision.txt", 1, 3);
    //GenerateMRsForSineOrCosLowPrecision(cosineSquared, std::filesystem::current_path().generic_string() + "/CosineTest.txt", 1, 3);
    //GenerateMRsForSineOrCosHighPrecision(cosineSquared, std::filesystem::current_path().generic_string() + "/CosineTestHighPrecision.txt", 1, 3);
    //GenerateMRsForPowLowPrecision(Pow, std::filesystem::current_path().generic_string() + "/PowerFuncTest.txt", 1, 1);
    //GenerateMRsForPowHighPrecision(Pow, std::filesystem::current_path().generic_string() + "/PowerFuncTestHighPrecision.txt", 1, 1);

    return 0;
}