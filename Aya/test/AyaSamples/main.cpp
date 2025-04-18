#include "TrigTest.hpp"
#include "PowTest.hpp"

int main() {
    GenerateMRsForSineOrCos(sine, std::filesystem::current_path().generic_string() + "/SineTest.txt", 1, 3);
    GenerateMRsForPow();

    return 0;
}