#include "LinalgTest.hpp"
#include "StandardMathTest.hpp"
#include "InputGenerator.hpp"
#include "Stopwatch.hpp"

inline void RunFullTest() {
#pragma region TrigTest_Sine_Cosine
    TIMED_CALL("SineSquared Test Generation", 
        GenerateMRsForDoubleDoubleArgFunc(SineSquared, equals,
            std::filesystem::current_path().generic_string() + "/Sine2Test.txt",
            1, 3,
            0, 1,
            {Aya::GenerateDoublesAsAny(10, 30, 90)},
            {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );
    
    TIMED_CALL("Sine Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(Sine, equals,
           std::filesystem::current_path().generic_string() + "/SineTest.txt",
           1, 3,
           0, 1,
           {Aya::GenerateDoublesAsAny(10, 30, 90)},
           {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );
    
    TIMED_CALL("Cosine Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(Cosine, equals,
           std::filesystem::current_path().generic_string() + "/CosineTest.txt",
           1, 3,
           0, 1,
           {Aya::GenerateDoublesAsAny(10, 30, 90)},
           {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );
    
    TIMED_CALL("CosineSquared Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(CosineSquared, equals,
            std::filesystem::current_path().generic_string() + "/Cosine2Test.txt",
            1, 3,
            0, 1,
            {Aya::GenerateDoublesAsAny(10, 30, 90)},
            {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );
    
    TIMED_CALL("SineSquared High Precision Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(SineSquared, equalsWithMorePrecision,
            std::filesystem::current_path().generic_string() + "/Sine2TestHighPrecision.txt",
            1, 3,
            0, 1,
            {Aya::GenerateDoublesAsAny(10, 30, 90)},
            {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );
    
    TIMED_CALL("CosineSquared High Precision Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(CosineSquared, equalsWithMorePrecision,
            std::filesystem::current_path().generic_string() + "/Cosine2TestHighPrecision.txt",
            1, 3,
            0, 1,
            {Aya::GenerateDoublesAsAny(10, 30, 90)},
            {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );
#pragma endregion
#pragma region TrigTest_Tan_Atan
    TIMED_CALL("Tan High Precision Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(TanFunc, equalsWithMorePrecision,
            std::filesystem::current_path().generic_string() + "/TanTestHighPrecision.txt",
            1, 3,
            0, 1,
            {Aya::GenerateDoublesAsAny(10, 30, 90)},
            {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );
    
    TIMED_CALL("Tan Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(TanFunc, equals,
            std::filesystem::current_path().generic_string() + "/TanTest.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 30, 90)},
            {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );

    TIMED_CALL("Atan High Precision Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(AtanFunc, equalsWithMorePrecision,
            std::filesystem::current_path().generic_string() + "/AtanTestHighPrecision.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 30, 90)},
            {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );
    
    TIMED_CALL("Atan Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(AtanFunc, equals,
            std::filesystem::current_path().generic_string() + "/AtanTest.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 30, 90)},
            {Aya::GenerateDoublesAsAny(10, 30, 90)})
    );
#pragma endregion
#pragma region TrigTest_Asin_Acos
    TIMED_CALL("Asin High Precision Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(AsinFunc, equalsWithMorePrecision,
            std::filesystem::current_path().generic_string() + "/AsinTestHighPrecision.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );
    
    TIMED_CALL("Asin Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(AsinFunc, equals,
            std::filesystem::current_path().generic_string() + "/AsinTest.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );
    
    TIMED_CALL("Acos High Precision Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(AcosFunc, equalsWithMorePrecision,
            std::filesystem::current_path().generic_string() + "/AcosTestHighPrecision.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );
    
    TIMED_CALL("Acos Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(AcosFunc, equals,
            std::filesystem::current_path().generic_string() + "/AcosTest.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );
#pragma endregion
#pragma region log_exp_root
    TIMED_CALL("Exp High Precision Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(ExpFunc, equalsWithMorePrecision,
            std::filesystem::current_path().generic_string() + "/ExpTestHighPrecision.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );
    
    TIMED_CALL("Exp Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(ExpFunc, equals,
            std::filesystem::current_path().generic_string() + "/ExpTest.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );

    TIMED_CALL("Log High Precision Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(LogFunc, equalsWithMorePrecision,
            std::filesystem::current_path().generic_string() + "/LogTestHighPrecision.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );
    
    TIMED_CALL("Log Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(LogFunc, equals,
            std::filesystem::current_path().generic_string() + "/LogTest.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );

    TIMED_CALL("Root High Precision Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(RootFunc, equalsWithMorePrecision,
            std::filesystem::current_path().generic_string() + "/RootTestHighPrecision.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );
    
    TIMED_CALL("Root Test Generation",
        GenerateMRsForDoubleDoubleArgFunc(RootFunc, equals,
            std::filesystem::current_path().generic_string() + "/RootTest.txt",
            1, 3,
            0, 0,
            {Aya::GenerateDoublesAsAny(10, 0, 0)},
            {Aya::GenerateDoublesAsAny(10, 0, 0)})
    );
#pragma endregion

#pragma region linalg
    TIMED_CALL("Matrix Multiplication Test Generation",
        GenerateMRsForMatrixMul(Vec2, vec2dEqual,
            std::filesystem::current_path().generic_string() + "/MatrixMulTest.txt",
            5, 2,
            0, 0,
            {Aya::Generate2dVectorsAsAny(5, 0, 60)},
            {Aya::Generate2dVectorsAsAny(5, 0, 60)})
    );

    TIMED_CALL("Matrix Multiplication High Precision Test Generation",
        GenerateMRsForMatrixMul(Vec2, vec2dEqualHighPrecision,
        std::filesystem::current_path().generic_string() + "/MatrixMulTestHighPrecision.txt",
        5, 2,
        0, 0,
        {Aya::Generate2dVectorsAsAny(5, 0, 60)},
        {Aya::Generate2dVectorsAsAny(5, 0, 60)})
    );
#pragma endregion

    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "All test generations completed successfully!" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

int main(void) {
    RunFullTest();
    return 0;
}