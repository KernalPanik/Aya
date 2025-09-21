#include "LinalgTest.hpp"
#include "StandardMathTest.hpp"
#include "InputGenerator.hpp"
#include "Stopwatch.hpp"
#include "MathMutants.hpp"

// Simple improvised Mutation Test for cmath functions
void RunMutationTest(const std::function<double(double)> &func, const std::function<double(double)> &mutatedFunc,
                     const char *funcDataFile, const char *mutatedDataFile,
                     size_t inputTransformChainLength, size_t outputTransformChainLength,
                     size_t leftValueIndex, size_t rightValueIndex,
                     const std::vector<std::vector<std::any> > &testInputs,
                     const std::vector<std::vector<std::any> > &validationInputs) {
    std::vector<Aya::MetamorphicRelation> MRs;
    GenerateMRsForDoubleDoubleArgFuncInternal(func, equals,
                                      std::filesystem::current_path().generic_string() + funcDataFile,
                                      inputTransformChainLength, outputTransformChainLength,
                                      leftValueIndex, rightValueIndex,
                                      testInputs,
                                      validationInputs,
                                      MRs);
    Aya::CalculateMRScore<double, double>(mutatedFunc, equals, MRs, validationInputs, leftValueIndex, rightValueIndex);
    Aya::ProduceMREvaluationReport(MRs, validationInputs, 28, 28,
        inputTransformChainLength, outputTransformChainLength,
        doubleTypeToString, std::filesystem::current_path().generic_string() + mutatedDataFile);
}

inline void RunMutationTests() {
    auto inputs = { Aya::GenerateDoublesAsAny(10, 30, 90) };
    auto validationInputs = { Aya::GenerateDoublesAsAny(10, 30, 90) };
    // SineSquared mutation tests
    TIMED_CALL("SineSquared vs SineSquared_Mutant1",
        RunMutationTest(SineSquared, Mutants::SineSquared_Mutant1,
            "/Sine2Test.txt",
            "/Sine2Test_M1.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("SineSquared vs SineSquared_Mutant2",
        RunMutationTest(SineSquared, Mutants::SineSquared_Mutant2,
            "/Sine2Test.txt",
            "/Sine2Test_M2.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("SineSquared vs SineSquared_Mutant3",
        RunMutationTest(SineSquared, Mutants::SineSquared_Mutant3,
            "/Sine2Test.txt",
            "/Sine2Test_M3.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("SineSquared vs SineSquared_Mutant4",
        RunMutationTest(SineSquared, Mutants::SineSquared_Mutant4,
            "/Sine2Test.txt",
            "/Sine2Test_M4.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("SineSquared vs SineSquared_Mutant5",
        RunMutationTest(SineSquared, Mutants::SineSquared_Mutant5,
            "/Sine2Test.txt",
            "/Sine2Test_M5.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );

    // Sine mutation tests
    TIMED_CALL("Sine vs Sine_Mutant1",
        RunMutationTest(Sine, Mutants::Sine_Mutant1,
            "/SineTest.txt",
            "/SineTest_M1.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("Sine vs Sine_Mutant2",
        RunMutationTest(Sine, Mutants::Sine_Mutant2,
            "/SineTest.txt",
            "/SineTest_M2.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("Sine vs Sine_Mutant3",
        RunMutationTest(Sine, Mutants::Sine_Mutant3,
            "/SineTest.txt",
            "/SineTest_M3.txt",
            1, 3,
            0, 1,
            { Aya::GenerateDoublesAsAny(10, 30, 90) },
            { Aya::GenerateDoublesAsAny(10, 30, 90) })
    );
    
    TIMED_CALL("Sine vs Sine_Mutant4",
        RunMutationTest(Sine, Mutants::Sine_Mutant4,
            "/SineTest.txt",
            "/SineTest_M4.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("Sine vs Sine_Mutant5",
        RunMutationTest(Sine, Mutants::Sine_Mutant5,
            "/SineTest.txt",
            "/SineTest_M5.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );

    // Cosine mutation tests
    TIMED_CALL("Cosine vs Cosine_Mutant1",
        RunMutationTest(Cosine, Mutants::Cosine_Mutant1,
            "/CosineTest.txt",
            "/CosineTest_M1.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("Cosine vs Cosine_Mutant2",
        RunMutationTest(Cosine, Mutants::Cosine_Mutant2,
            "/CosineTest.txt",
            "/CosineTest_M2.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("Cosine vs Cosine_Mutant3",
        RunMutationTest(Cosine, Mutants::Cosine_Mutant3,
            "/CosineTest.txt",
            "/CosineTest_M3.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("Cosine vs Cosine_Mutant4",
        RunMutationTest(Cosine, Mutants::Cosine_Mutant4,
            "/CosineTest.txt",
            "/CosineTest_M4.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("Cosine vs Cosine_Mutant5",
        RunMutationTest(Cosine, Mutants::Cosine_Mutant5,
            "/CosineTest.txt",
            "/CosineTest_M5.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );

    // CosineSquared mutation tests
    TIMED_CALL("CosineSquared vs CosineSquared_Mutant1",
        RunMutationTest(CosineSquared, Mutants::CosineSquared_Mutant1,
            "/Cosine2Test.txt",
            "/Cosine2Test_M1.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("CosineSquared vs CosineSquared_Mutant2",
        RunMutationTest(CosineSquared, Mutants::CosineSquared_Mutant2,
            "/Cosine2Test.txt",
            "/Cosine2Test_M2.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("CosineSquared vs CosineSquared_Mutant3",
        RunMutationTest(CosineSquared, Mutants::CosineSquared_Mutant3,
            "/Cosine2Test.txt",
            "/Cosine2Test_M3.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("CosineSquared vs CosineSquared_Mutant4",
        RunMutationTest(CosineSquared, Mutants::CosineSquared_Mutant4,
            "/Cosine2Test.txt",
            "/Cosine2Test_M4.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("CosineSquared vs CosineSquared_Mutant5",
        RunMutationTest(CosineSquared, Mutants::CosineSquared_Mutant5,
            "/Cosine2Test.txt",
            "/Cosine2Test_M5.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );

    auto rootInputs = { Aya::GenerateDoublesAsAny(1, 10, 100) };
    auto rootValidationInputs = { Aya::GenerateDoublesAsAny(1, 10, 100) };

    // RootFunc mutation tests
    TIMED_CALL("RootFunc vs RootFunc_Mutant1",
        RunMutationTest(RootFunc, Mutants::RootFunc_Mutant1,
            "/RootTest.txt",
            "/RootTest_M1.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );
    
    TIMED_CALL("RootFunc vs RootFunc_Mutant2",
        RunMutationTest(RootFunc, Mutants::RootFunc_Mutant2,
            "/RootTest.txt",
            "/RootTest_M2.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );
    
    TIMED_CALL("RootFunc vs RootFunc_Mutant3",
        RunMutationTest(RootFunc, Mutants::RootFunc_Mutant3,
            "/RootTest.txt",
            "/RootTest_M3.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );
    
    TIMED_CALL("RootFunc vs RootFunc_Mutant4",
        RunMutationTest(RootFunc, Mutants::RootFunc_Mutant4,
            "/RootTest.txt",
            "/RootTest_M4.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );
    
    TIMED_CALL("RootFunc vs RootFunc_Mutant5",
        RunMutationTest(RootFunc, Mutants::RootFunc_Mutant5,
            "/RootTest.txt",
            "/RootTest_M5.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );

    // LogFunc mutation tests
    TIMED_CALL("LogFunc vs LogFunc_Mutant1",
        RunMutationTest(LogFunc, Mutants::LogFunc_Mutant1,
            "/LogTest.txt",
            "/LogTest_M1.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );
    
    TIMED_CALL("LogFunc vs LogFunc_Mutant2",
        RunMutationTest(LogFunc, Mutants::LogFunc_Mutant2,
            "/LogTest.txt",
            "/LogTest_M2.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );
    
    TIMED_CALL("LogFunc vs LogFunc_Mutant3",
        RunMutationTest(LogFunc, Mutants::LogFunc_Mutant3,
            "/LogTest.txt",
            "/LogTest_M3.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );
    
    TIMED_CALL("LogFunc vs LogFunc_Mutant4",
        RunMutationTest(LogFunc, Mutants::LogFunc_Mutant4,
            "/LogTest.txt",
            "/LogTest_M4.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );
    
    TIMED_CALL("LogFunc vs LogFunc_Mutant5",
        RunMutationTest(LogFunc, Mutants::LogFunc_Mutant5,
            "/LogTest.txt",
            "/LogTest_M5.txt",
            1, 3,
            0, 1,
            rootInputs,
            rootValidationInputs)
    );

    // TanFunc mutation tests
    TIMED_CALL("TanFunc vs TanFunc_Mutant1",
        RunMutationTest(TanFunc, Mutants::TanFunc_Mutant1,
            "/TanTest.txt",
            "/TanTest_M1.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("TanFunc vs TanFunc_Mutant2",
        RunMutationTest(TanFunc, Mutants::TanFunc_Mutant2,
            "/TanTest.txt",
            "/TanTest_M2.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("TanFunc vs TanFunc_Mutant3",
        RunMutationTest(TanFunc, Mutants::TanFunc_Mutant3,
            "/TanTest.txt",
            "/TanTest_M3.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("TanFunc vs TanFunc_Mutant4",
        RunMutationTest(TanFunc, Mutants::TanFunc_Mutant4,
            "/TanTest.txt",
            "/TanTest_M4.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("TanFunc vs TanFunc_Mutant5",
        RunMutationTest(TanFunc, Mutants::TanFunc_Mutant5,
            "/TanTest.txt",
            "/TanTest_M5.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );


    auto asinInput = { Aya::GenerateDoublesAsAny(10, -1, 1) };
    auto asinValidationInput = { Aya::GenerateDoublesAsAny(10, -1, 1) };
    // AsinFunc mutation tests
    TIMED_CALL("AsinFunc vs AsinFunc_Mutant1",
        RunMutationTest(AsinFunc, Mutants::AsinFunc_Mutant1,
            "/AsinTest.txt",
            "/AsinTest_M1.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );
    
    TIMED_CALL("AsinFunc vs AsinFunc_Mutant2",
        RunMutationTest(AsinFunc, Mutants::AsinFunc_Mutant2,
            "/AsinTest.txt",
            "/AsinTest_M2.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );
    
    TIMED_CALL("AsinFunc vs AsinFunc_Mutant3",
        RunMutationTest(AsinFunc, Mutants::AsinFunc_Mutant3,
            "/AsinTest.txt",
            "/AsinTest_M3.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );
    
    TIMED_CALL("AsinFunc vs AsinFunc_Mutant4",
        RunMutationTest(AsinFunc, Mutants::AsinFunc_Mutant4,
            "/AsinTest.txt",
            "/AsinTest_M4.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );
    
    TIMED_CALL("AsinFunc vs AsinFunc_Mutant5",
        RunMutationTest(AsinFunc, Mutants::AsinFunc_Mutant5,
            "/AsinTest.txt",
            "/AsinTest_M5.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );

    // AcosFunc mutation tests
    TIMED_CALL("AcosFunc vs AcosFunc_Mutant1",
        RunMutationTest(AcosFunc, Mutants::AcosFunc_Mutant1,
            "/AcosTest.txt",
            "/AcosTest_M1.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );
    
    TIMED_CALL("AcosFunc vs AcosFunc_Mutant2",
        RunMutationTest(AcosFunc, Mutants::AcosFunc_Mutant2,
            "/AcosTest.txt",
            "/AcosTest_M2.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );
    
    TIMED_CALL("AcosFunc vs AcosFunc_Mutant3",
        RunMutationTest(AcosFunc, Mutants::AcosFunc_Mutant3,
            "/AcosTest.txt",
            "/AcosTest_M3.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );
    
    TIMED_CALL("AcosFunc vs AcosFunc_Mutant4",
        RunMutationTest(AcosFunc, Mutants::AcosFunc_Mutant4,
            "/AcosTest.txt",
            "/AcosTest_M4.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );
    
    TIMED_CALL("AcosFunc vs AcosFunc_Mutant5",
        RunMutationTest(AcosFunc, Mutants::AcosFunc_Mutant5,
            "/AcosTest.txt",
            "/AcosTest_M5.txt",
            1, 3,
            0, 1,
            asinInput,
            asinValidationInput)
    );

    // AtanFunc mutation tests
    TIMED_CALL("AtanFunc vs AtanFunc_Mutant1",
        RunMutationTest(AtanFunc, Mutants::AtanFunc_Mutant1,
            "/AtanTest.txt",
            "/AtanTest_M1.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("AtanFunc vs AtanFunc_Mutant2",
        RunMutationTest(AtanFunc, Mutants::AtanFunc_Mutant2,
            "/AtanTest.txt",
            "/AtanTest_M2.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("AtanFunc vs AtanFunc_Mutant3",
        RunMutationTest(AtanFunc, Mutants::AtanFunc_Mutant3,
            "/AtanTest.txt",
            "/AtanTest_M3.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("AtanFunc vs AtanFunc_Mutant4",
        RunMutationTest(AtanFunc, Mutants::AtanFunc_Mutant4,
            "/AtanTest.txt",
            "/AtanTest_M4.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );
    
    TIMED_CALL("AtanFunc vs AtanFunc_Mutant5",
        RunMutationTest(AtanFunc, Mutants::AtanFunc_Mutant5,
            "/AtanTest.txt",
            "/AtanTest_M5.txt",
            1, 3,
            0, 1,
            inputs,
            validationInputs)
    );

    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "All mutation tests completed successfully!" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

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

#pragma region MutantTesting
    // Test SineSquared mutants
    auto sineSquaredMutants = Mutants::GetSineSquaredMutants();
    auto sineSquaredMutantNames = Mutants::GetSineSquaredMutantNames();
    for (size_t i = 0; i < sineSquaredMutants.size(); ++i) {
        TIMED_CALL(sineSquaredMutantNames[i] + " Test Generation",
            GenerateMRsForDoubleDoubleArgFunc(sineSquaredMutants[i], equals,
                std::filesystem::current_path().generic_string() + "/" + sineSquaredMutantNames[i] + "Test.txt",
                1, 3,
                0, 1,
                {Aya::GenerateDoublesAsAny(10, 30, 90)},
                {Aya::GenerateDoublesAsAny(10, 30, 90)})
        );
    }

    // Test Sine mutants
    auto sineMutants = Mutants::GetSineMutants();
    auto sineMutantNames = Mutants::GetSineMutantNames();
    for (size_t i = 0; i < sineMutants.size(); ++i) {
        TIMED_CALL(sineMutantNames[i] + " Test Generation",
            GenerateMRsForDoubleDoubleArgFunc(sineMutants[i], equals,
                std::filesystem::current_path().generic_string() + "/" + sineMutantNames[i] + "Test.txt",
                1, 3,
                0, 1,
                {Aya::GenerateDoublesAsAny(10, 30, 90)},
                {Aya::GenerateDoublesAsAny(10, 30, 90)})
        );
    }

    // Test Cosine mutants
    auto cosineMutants = Mutants::GetCosineMutants();
    auto cosineMutantNames = Mutants::GetCosineMutantNames();
    for (size_t i = 0; i < cosineMutants.size(); ++i) {
        TIMED_CALL(cosineMutantNames[i] + " Test Generation",
            GenerateMRsForDoubleDoubleArgFunc(cosineMutants[i], equals,
                std::filesystem::current_path().generic_string() + "/" + cosineMutantNames[i] + "Test.txt",
                1, 3,
                0, 1,
                {Aya::GenerateDoublesAsAny(10, 30, 90)},
                {Aya::GenerateDoublesAsAny(10, 30, 90)})
        );
    }

    // Test CosineSquared mutants
    auto cosineSquaredMutants = Mutants::GetCosineSquaredMutants();
    auto cosineSquaredMutantNames = Mutants::GetCosineSquaredMutantNames();
    for (size_t i = 0; i < cosineSquaredMutants.size(); ++i) {
        TIMED_CALL(cosineSquaredMutantNames[i] + " Test Generation",
            GenerateMRsForDoubleDoubleArgFunc(cosineSquaredMutants[i], equals,
                std::filesystem::current_path().generic_string() + "/" + cosineSquaredMutantNames[i] + "Test.txt",
                1, 3,
                0, 1,
                {Aya::GenerateDoublesAsAny(10, 30, 90)},
                {Aya::GenerateDoublesAsAny(10, 30, 90)})
        );
    }
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
    RunMutationTests();
    return 0;
}