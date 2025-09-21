# Aya

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

**Aya** is a powerful C++ library for automated Metamorphic Relation (MR) generation and testing. It systematically discovers mathematical relationships in functions by applying input and output transformations, making it invaluable for software testing, validation, and mathematical analysis.

## Features

- **Automated MR Discovery**: Automatically generates metamorphic relations for mathematical functions
- **Flexible Transformation System**: Support for custom input and output transformations
- **Chain Composition**: Combine multiple transformations to discover complex relationships
- **Validation Framework**: Built-in scoring and validation system for generated MRs
- **Header-Only Library**: Easy integration with single header include
- **Comprehensive Reporting**: Detailed analysis and success rate reporting

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [Step-by-Step Guide: pow(x, e) Example](#step-by-step-guide-powx-e-example)
- [Core Concepts](#core-concepts)
- [API Reference](#api-reference)
- [Examples](#examples)
- [Building from Source](#building-from-source)
- [Contributing](#contributing)
- [License](#license)

## Installation

Aya is distributed as a header-only library. Simply add the `include` folder to your project:

```cpp
#include "Aya.hpp"
```

### Requirements

- C++17 or later
- CMake 3.10+ (for building samples)
- Standard math library support

## Quick Start

Here's a minimal example to get you started:

```cpp
#include "Aya.hpp"
#include <cmath>

// Define your function
inline double Square(const double x) {
    return x * x;
}

// Define transformers
inline void MultiplyBy2(double &x) { x *= 2.0; }
inline void MultiplyBy4(double &x) { x *= 4.0; }

// Generate MRs
void GenerateSquareMRs() {
    // Create transformers
    auto inputTransformers = Aya::TransformBuilder<double>()
        .GetTransformers({MultiplyBy2}, {"MultiplyBy2"});
    
    // Set up MR builder and search for relations
    // ... (see full example below)
}
```

## Step-by-Step Guide: pow(x, e) Example

This comprehensive guide demonstrates how to generate Metamorphic Relations for the `pow(x, e)` function.

### Step 1: Setup and Headers

```cpp
#include "Aya.hpp"
#include <cmath>
#include <vector>
#include <functional>
```

### Step 2: Define Target Function

```cpp
// For single-argument example (pow with fixed exponent)
inline double PowSquare(const double x) {
    return pow(x, 2.0);  // Square function as example
}

// Equality checker with floating-point tolerance
inline bool equals(const double x, const double y) {
    return fabs(x - y) < 1e-6 || (isnan(x) && isnan(y));
}
```

### Step 3: Create Transformation Functions

```cpp
// Input transformers - modify the input value
inline void MultiplyBy2(double &x) { x *= 2.0; }
inline void MultiplyBy3(double &x) { x *= 3.0; }
inline void AddConstant(double &x, double val) { x += val; }
inline void MultiplyConstant(double &x, double val) { x *= val; }

// Output transformers - modify the expected output
inline void MultiplyBy4(double &x) { x *= 4.0; }  // Since (2x)² = 4x²
inline void MultiplyBy9(double &x) { x *= 9.0; }  // Since (3x)² = 9x²
```

### Step 4: Build Transformer Collections

```cpp
// Define single-argument transformers
const std::vector<std::function<void(double &)>> singleArgTransformers = {
    MultiplyBy2, MultiplyBy3, MultiplyBy4, MultiplyBy9
};
const std::vector<std::string> singleArgNames = {
    "MultiplyBy2", "MultiplyBy3", "MultiplyBy4", "MultiplyBy9"
};

// Define parameterized transformers
const std::vector<std::function<void(double &, double)>> paramTransformers = {
    AddConstant, MultiplyConstant
};
const std::vector<std::string> paramNames = {
    "AddConstant", "MultiplyConstant"
};

// Create argument pools for parameterized transformers
std::vector<std::vector<std::tuple<double>>> argumentPool;
argumentPool.push_back({{1.0}, {2.0}, {-1.0}});  // AddConstant arguments
argumentPool.push_back({{2.0}, {3.0}, {0.5}});   // MultiplyConstant arguments
```

### Step 5: Initialize Transformer Pools

```cpp
// Build input transformers
auto singleInputTransformers = Aya::TransformBuilder<double>()
    .GetTransformers(singleArgTransformers, singleArgNames);

auto paramInputTransformers = Aya::TransformBuilder<double, double>()
    .GetTransformers(paramTransformers, paramNames, argumentPool);

// Combine all input transformers
std::vector<std::shared_ptr<Aya::ITransformer>> inputTransformers;
inputTransformers.insert(inputTransformers.end(), 
                        singleInputTransformers.begin(), singleInputTransformers.end());
inputTransformers.insert(inputTransformers.end(), 
                        paramInputTransformers.begin(), paramInputTransformers.end());

// Create output transformers (reuse input transformers)
std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformers = inputTransformers;
```

### Step 6: Configure Input Transformer Pool

```cpp
// Map transformers to input indices (index 0 for single-argument function)
std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
inputTransformerPool.insert({0, inputTransformers});

// Define variable transformer indices for parameterized transformers
std::vector<std::vector<size_t>> variableTransformerIndices;
for (size_t i = 0; i < paramInputTransformers.size(); ++i) {
    variableTransformerIndices.push_back({0});  // Use input[0] as parameter source
}
for (size_t i = 0; i < singleInputTransformers.size(); ++i) {
    variableTransformerIndices.push_back({});   // No parameters needed
}
```

### Step 7: Generate Test Data

```cpp
// Test inputs for MR discovery
std::vector<std::vector<std::any>> testInputs = {
    {1.0, 2.0, 3.0, 4.0, 5.0, 0.5, 1.5, 2.5}
};

// Validation inputs for MR scoring (should be different from test inputs)
std::vector<std::vector<std::any>> validationInputs = {
    {1.1, 2.1, 3.1, 4.1, 5.1, 0.6, 1.6, 2.6}
};
```

### Step 8: Create MRBuilder and Search

```cpp
// Initialize the MR builder
auto mrBuilder = Aya::MRBuilder<double, double, double>(
    PowSquare,                      // Target function
    equals,                         // Equality comparer
    inputTransformerPool,           // Input transformers mapped by index
    outputTransformers,             // Output transformers
    0,                              // Left value index for comparison
    0,                              // Right value index for comparison
    outputTransformers,             // Variable output transformers
    variableTransformerIndices      // Variable transformer configuration
);

// Enable automatic output transformation discovery
mrBuilder.SetEnableImplicitOutputTransforms(true);

// Search for metamorphic relations
std::vector<Aya::MetamorphicRelation> foundMRs;
size_t totalMatches = 0;
size_t inputChainLength = 1;    // Single transformation per input chain
size_t outputChainLength = 2;   // Up to 2 transformations per output chain

mrBuilder.SearchForMRs(testInputs, inputChainLength, outputChainLength, 
                      totalMatches, foundMRs);
```

### Step 9: Validate and Score MRs

```cpp
// Calculate success rates using validation data
Aya::CalculateMRScore<double, double, double>(
    PowSquare,          // Function to validate against
    equals,             // Equality comparer
    foundMRs,           // MRs to validate
    validationInputs,   // Validation dataset
    0,                  // Left comparison index
    0,                  // Right comparison index
    true                // Override arguments flag
);
```

### Step 10: Generate Comprehensive Report

```cpp
// Create detailed evaluation report
std::string reportFile = "pow_function_metamorphic_relations.txt";

Aya::ProduceMREvaluationReport(
    foundMRs,                       // Discovered MRs
    validationInputs,               // Validation inputs used
    inputTransformers.size(),       // Total input transformers available
    outputTransformers.size(),      // Total output transformers available
    inputChainLength,               // Input transformation chain length
    outputChainLength,              // Output transformation chain length
    [](const std::any& val) {       // Value-to-string converter
        return std::to_string(std::any_cast<double>(val));
    },
    reportFile,                     // Output file path
    0.8f                           // Minimum success rate threshold (optional)
);

// Display results
std::cout << "MR Generation Complete!" << std::endl;
std::cout << "Found " << foundMRs.size() << " metamorphic relations" << std::endl;
std::cout << "Total potential matches evaluated: " << totalMatches << std::endl;
std::cout << "Detailed report saved to: " << reportFile << std::endl;
```

### Expected Results

The tool will discover metamorphic relations such as:

- `MultiplyBy2(input[0]) === MultiplyBy4(initialState[0])` 
  - **Meaning**: If you double the input to `pow(x,2)`, the output should be quadrupled
  - **Mathematical basis**: `(2x)² = 4x²`

- `MultiplyBy3(input[0]) === MultiplyBy9(initialState[0])`
  - **Meaning**: If you triple the input, the output should be multiplied by 9
  - **Mathematical basis**: `(3x)² = 9x²`

### Complete Working Example

```cpp
void GeneratePowMRs() {
    // ... combine all steps above ...
    
    std::cout << "Metamorphic Relations for pow(x,2):" << std::endl;
    for (const auto& mr : foundMRs) {
        if (mr.LastSuccessRate > 0.9f) {  // Show high-confidence MRs
            std::cout << mr.ToString() << std::endl;
        }
    }
}
```

## Core Concepts

### Metamorphic Relations
A metamorphic relation defines a relationship between multiple executions of a program. For a function `f`, an MR might state: "if input `x` is transformed by `T₁` to get `x'`, then `f(x')` should be related to `f(x)` by transformation `T₂`".

### Transformers
- **Input Transformers**: Modify function inputs (e.g., multiply by 2, add constant)
- **Output Transformers**: Define expected output relationships (e.g., square result, multiply by constant)
- **Parameterized Transformers**: Accept additional arguments for flexible transformations

### MRBuilder
The core engine that:
1. Generates transformation chains
2. Applies them to test inputs
3. Validates relationships
4. Scores MR effectiveness

## API Reference

### Key Classes

- `Aya::MRBuilder<T, U, Args...>` - Main MR generation engine
- `Aya::TransformBuilder<T, Args...>` - Creates transformer instances
- `Aya::MetamorphicRelation` - Represents a discovered MR
- `Aya::ITransformer` - Base interface for all transformers

### Key Functions

- `SearchForMRs()` - Discovers metamorphic relations
- `CalculateMRScore()` - Validates MRs against test data
- `ProduceMREvaluationReport()` - Generates detailed reports

## Building from Source

```bash
# Clone the repository
git clone https://github.com/KernalPanik/Aya.git
cd Aya

# Build with CMake
mkdir build && cd build
cmake ..
make

# Run samples
./samples/math/math_sample
./samples/stl/stl_sample
```

### Build Requirements

- CMake 3.10 or later
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Standard library with `<cmath>`, `<vector>`, `<functional>` support

## Examples

Check out the `samples/` directory for complete examples:

- `samples/math/` - Mathematical function MR generation (trigonometric, exponential, etc.)
- `samples/stl/` - STL container MR generation (vectors, algorithms, etc.)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.