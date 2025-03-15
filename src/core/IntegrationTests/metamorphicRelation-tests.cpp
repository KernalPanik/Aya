#include "metamorphicRelation-tests.h"
#include "../../../test/Framework/testRunnerUtils.h"
#include "src/Common/tuple-utils.h"
#include "src/core/Modules/Transformer/transformer.h"
#include "src/core/Modules/Transformer/TransformBuilder.hpp"
#include "src/core/Modules/MRGenerator/TestContext.hpp"
#include "src/core/Modules/MRGenerator/MRBuilder.hpp"

#include <iostream>
#include <map>
#include <cmath>

using namespace Core;

/*
Why wrap pow() in poww()?
cmath pow() contains many overloads. std::function<> template within TestableFunctionBase cannot deduce which
overload to use. It's easy to mitigate via slim wrappers like poww, for example.
*/

#pragma region Helper functions
double poww(double x, double y) {
    return pow(x, y);
}

void Add(double& b, double val) {
    b += val;
}

void Mul(double& b, double val) {
    b *= val;
}

void Sub(double& b, double val) {
     b -= val;
}

void Div(double& b, double val) {
    b /= val;
}

void Noop(double& b, double val) {}

#pragma endregion
void MR_SimpleConstructionTest() {
    // MR: pow(x, y + 1) == pow(x, y) * x
#pragma region Double Transformers
    // TODO: See if TransformBuilder class can manage more than just packing individual func to args
    std::vector<std::vector<double>> transformerArgumentPool;
    std::vector<std::vector<double>> transformerArgumentPool1;
    const std::vector<std::function<void(double&, double)>> funcs = {Div, Sub, Mul, Add, Noop};
    const std::vector<std::function<void(double&, double)>> funcsForOutput = {Div, Sub, Mul, Add};

    transformerArgumentPool.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool.push_back({0.0});

    transformerArgumentPool1.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool1.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool1.push_back({1.0, 2.0, -1.0});
    transformerArgumentPool1.push_back({1.0, 2.0, -1.0});

    std::vector<std::shared_ptr<Aya::ITransformer>> doubleTransformers = Aya::TransformBuilder<double, double>().GetTransformers(funcs, transformerArgumentPool);
    std::vector<std::shared_ptr<Aya::ITransformer>> doubleTransformersForOutput = Aya::TransformBuilder<double, double>().GetTransformers(funcsForOutput, transformerArgumentPool);

#pragma endregion

#pragma region CartesianIterator builders
    // Map index to possible input Transformers by type
    std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
    inputTransformerPool.insert({0, doubleTransformers});
    inputTransformerPool.insert({1, doubleTransformers});

    std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> outputTransformerPool;
    outputTransformerPool.insert({0, doubleTransformersForOutput});

#pragma endregion

    std::vector<std::shared_ptr<Core::IMRContext>> goodContexts; // Validation returned true for them -- potential MRs
    size_t overallMatchCount = 0;
    auto mrBuilder = Aya::MRBuilder<double, double, double, double>(poww,
        inputTransformerPool, outputTransformerPool, funcsForOutput, {0, 1}, 0);
    std::vector<std::vector<std::any>> testedInputs;
    testedInputs.push_back({10.0, 11.0, 12.0});
    testedInputs.push_back({2.0, 3.0, 4.0});
    mrBuilder.SearchForMRs(testedInputs, 1, 1, overallMatchCount);
    std::cout << "Found " << overallMatchCount << " possible MRs" << std::endl;

#pragma region TransformerPrep


#pragma endregion
    TEST_EXPECT(2 == 2); // TODO: After API is settled and MRs are produced, update this check to match MR count for Pow()
}