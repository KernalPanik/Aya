# Aya

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)

Header-only C++20 library for automatic discovery of **metamorphic relations** (MRs) for pure functions. Given a function under test, a set of input/output transformers, and sample inputs, Aya searches the combinatorial space of transformer compositions and returns the relations that hold across all validation inputs.

## Headers

| Header | Description |
|---|---|
| `aya.hpp` | Core library — `MREngine`, transformers, Cartesian iterator, scoring, reporting |
| `aya_parallel.hpp` | Drop-in parallel extension — `ParallelMREngine` with configurable thread count |

## Requirements

- C++20 compiler:
  - GCC 12+
  - Clang 18+ (Linux — earlier versions are incompatible with GCC 14 libstdc++ headers)
  - Apple Clang 15+ (macOS)
- `-pthread` for the parallel header
- Docker + Docker Compose (optional, for containerised testing)

## Local Testing

### With Docker (recommended)

Builds and runs the test suite under both GCC and Clang in isolated containers, and compiles the benchmark and the jpeg test projects:

```bash
docker compose up --build
```

### Without Docker

Compile and run the test suite directly:

```bash
# GCC
g++ -std=c++20 -O2 -pthread -I src -o aya_tests src/aya_tests.cpp && ./aya_tests

# Clang (macOS)
clang++ -std=c++20 -O2 -pthread -I src -o aya_tests src/aya_tests.cpp && ./aya_tests
```

Alternatively, use `run_tests.sh` and `run_all_benchmarks.sh` scripts.

## Quick Start

### Building and running the test suite

```bash
clang++ -std=c++20 -O2 -I src -o aya_tests src/aya_tests.cpp
./aya_tests
```

The test suite covers:

1. Trigonometric functions (sin, cos, tan, asin, acos, atan, sin^2, cos^2)
2. Exponential, logarithm, and square root
3. `pow(base, exp)` with state-aware (variable) output transforms
4. `std::vector<int>` push/pop roundtrips
5. 2D rotation matrix compositions
6. `std::string` permutation invariance under sort
7. `Point{x,y}` struct — distance invariance under reflection/rotation
8. `std::set<int>` algebra properties
9. `std::map<string,int>` insert/erase roundtrips
10. Mutation testing — verifying MR reactivity (sin vs cos mutant)
11. Heap property preservation
12. Edge cases (empty pools, domain errors, minimal config)

### Building and running the parallel benchmark

```bash
clang++ -std=c++20 -O2 -pthread -I src -o aya_par_bench projects/benchmarks/aya_parallel_benchmark.cpp
./aya_par_bench
```

## Usage

### 1. Define transformers

```cpp
#include "aya.hpp"

// No-arg transformer: void(T&)
auto negate = Aya::MakeTransformer<double>(
    "Negate", std::function<void(double&)>([](double& x) { x = -x; }));

// Transformer with bound arg: void(T&, Args...)
auto add_pi = Aya::MakeTransformer<double, double>(
    "AddPi", std::function<void(double&, double)>(
        [](double& x, double c) { x += c; }), M_PI);
```

### 2. Configure the engine

```cpp
auto engine = Aya::MREngine<double, double>(
    [](double x) -> double { return std::sin(x); },        // function under test
    [](double a, double b) { return std::abs(a-b) < 1e-9; } // equality comparator
);

engine.AddInputTransformers(0, {negate, add_pi});
engine.AddOutputTransformers({negate});
```

### 3. Search

```cpp
auto mrs = engine.Search(
    /*sample_inputs*/  {{std::any(0.3), std::any(1.1), std::any(2.5)}},
    /*verify_inputs*/  {{std::any(0.2), std::any(0.9), std::any(3.0)}},
    /*input_chain_len*/  1,
    /*output_chain_len*/ 1,
    /*left_idx*/  0, /*right_idx*/ 0,
    /*min_success*/ 1.0f
);

Aya::DumpMRsToStdout(mrs);
```

### 4. Parallel search (drop-in replacement)

```cpp
#include "aya_parallel.hpp"

auto engine = Aya::ParallelMREngine<double, double>(
    func, comparator,
    8  // thread count (0 = hardware_concurrency)
);
// Same AddInputTransformers / AddOutputTransformers / Search API
```

## Benchmark Results

Measured on Apple M2 Pro (10 cores), macOS, compiled with `clang++ -std=c++20 -O2 -pthread`.

Pool: 28 transformers (12 no-arg + 16 with bound args). 10 sample / 10 verify inputs.

### sin(x)

| Config | Mode | Threads | Raw MRs | Unique MRs | Time (ms) | Speedup |
|--------|------|---------|---------|------------|-----------|---------|
| 1-2 | Sequential | 1 | 2,360 | 116 | 188.0 | 1.00x |
| 1-2 | Parallel | 2 | 2,360 | 116 | 149.3 | 1.26x |
| 1-2 | Parallel | 4 | 2,360 | 116 | 107.1 | 1.76x |
| 1-2 | Parallel | 8 | 2,360 | 116 | 75.2 | 2.50x |
| 2-2 | Sequential | 1 | 40,180 | 1,849 | 4,959.3 | 1.00x |
| 2-2 | Parallel | 2 | 40,180 | 1,849 | 2,897.1 | 1.71x |
| 2-2 | Parallel | 4 | 40,180 | 1,849 | 1,874.4 | 2.65x |
| 2-2 | Parallel | 8 | 40,180 | 1,849 | 1,317.6 | 3.76x |

### asin(x)

| Config | Mode | Threads | Raw MRs | Unique MRs | Time (ms) | Speedup |
|--------|------|---------|---------|------------|-----------|---------|
| 1-2 | Sequential | 1 | 177,320 | 7,136 | 721.8 | 1.00x |
| 1-2 | Parallel | 4 | 177,320 | 7,136 | 306.6 | 2.35x |
| 1-2 | Parallel | 8 | 177,320 | 7,136 | 278.9 | 2.59x |
| 2-2 | Sequential | 1 | 3,168,580 | 128,218 | 14,469.6 | 1.00x |
| 2-2 | Parallel | 4 | 3,168,580 | 128,218 | 4,792.6 | 3.02x |
| 2-2 | Parallel | 8 | 3,168,580 | 128,218 | 3,526.0 | 4.10x |

### sin(x) / asin(x) — config 3-1 (large input chain)

| Function | Mode | Threads | Raw MRs | Unique MRs | Time (ms) | Speedup |
|----------|------|---------|---------|------------|-----------|---------|
| sin | Sequential | 1 | 11,350 | 1,135 | 2,040.4 | 1.00x |
| sin | Parallel | 4 | 11,350 | 1,135 | 996.4 | 2.05x |
| sin | Parallel | 8 | 11,350 | 1,135 | 730.1 | 2.79x |
| asin | Sequential | 1 | 1,716,330 | 108,429 | 7,252.2 | 1.00x |
| asin | Parallel | 4 | 1,716,330 | 108,429 | 2,549.9 | 2.84x |
| asin | Parallel | 8 | 1,716,330 | 108,429 | 2,018.4 | 3.59x |

Key observations:
- Unique MR counts are identical between sequential and parallel — correctness is preserved
- Speedup scales well with thread count, reaching **3-4x at 8 threads** on heavier configs
- Lighter workloads (config 1-2) show less speedup due to thread overhead dominating

## Repository Structure

Apart from the library source code in the `src` directory, `projects` folder contains sample projects used for more extensive checks. Benchmark project tests the performance and parallel speedup of the MR search, while `aya-jpeg` matches the JPG compression tests mentioned in the paper.