# ForkCombineTransformer

## The Problem

Aya's default transformer chains apply mutations **sequentially** to a single value. When transformer A runs before transformer B, B sees A's already-mutated output — not the original input. This means compound relationships like `sin(x) / cos(x) == tan(x)` are invisible to the engine, because by the time the division happens, the original `sin(x)` and `cos(x)` values are gone.

## The Solution

`ForkCombineTransformer` **snapshots** the input, applies two sub-transformers to independent copies, then combines the results:

```
         input
        /     \
    copy_L   copy_R
      |         |
    left_    right_
      |         |
    val_L    val_R
        \     /
     combiner_(val_L, val_R)
           |
        output
```

Both sub-transformers see the **original** input. No sequential mutation leakage.

## API

### `ForkCombineTransformer<T>`

The core class. Implements `ITransformer`.

```cpp
template <typename T>
class ForkCombineTransformer final : public ITransformer {
public:
    using Combiner = std::function<T(T, T)>;

    ForkCombineTransformer(
        TransformerPtr left,
        TransformerPtr right,
        Combiner combiner,
        std::string combiner_name
    );
};
```

### `MakeForkCombine<T>`

Convenience factory for a single fork-combine transformer.

```cpp
template <typename T>
TransformerPtr MakeForkCombine(
    TransformerPtr left,
    TransformerPtr right,
    std::function<T(T, T)> combiner,
    std::string combiner_name
);
```

### `MakeForkCombinePool<T>`

Batch-create fork-combine transformers from all pairwise combinations of primitives and combiners. Returns `O(N^2 * C)` transformers (skipping `i == j`).

```cpp
template <typename T>
std::vector<TransformerPtr> MakeForkCombinePool(
    const std::vector<TransformerPtr>& primitives,
    const std::vector<std::pair<std::string, std::function<T(T, T)>>>& combiners
);
```

## Usage Examples

### Manual Construction — tan = sin / cos

```cpp
auto sin_tf = Aya::MakeTransformer<double>("Sin", [](double& x){ x = std::sin(x); });
auto cos_tf = Aya::MakeTransformer<double>("Cos", [](double& x){ x = std::cos(x); });

std::function<double(double, double)> div_fn = [](double a, double b) { return a / b; };
auto tan_fc = Aya::MakeForkCombine<double>(sin_tf, cos_tf, div_fn, "Div");

// tan_fc->ToString("x", 0) => "Div( Sin(x), Cos(x) )"
```

### Pool Generation

```cpp
std::vector<Aya::TransformerPtr> primitives = {
    Aya::MakeTransformer<double>("Double",  [](double& x){ x *= 2; }),
    Aya::MakeTransformer<double>("Negate",  [](double& x){ x = -x; }),
    Aya::MakeTransformer<double>("Square",  [](double& x){ x *= x; }),
    Aya::MakeTransformer<double>("PlusOne", [](double& x){ x += 1; }),
};

std::vector<std::pair<std::string, std::function<double(double,double)>>> combiners = {
    {"Add", [](double a, double b){ return a + b; }},
    {"Mul", [](double a, double b){ return a * b; }},
};

auto fc_pool = Aya::MakeForkCombinePool<double>(primitives, combiners);
// 4 primitives * 3 partners * 2 combiners = 24 fork-combine transformers

// Append to existing pool
base_pool.insert(base_pool.end(), fc_pool.begin(), fc_pool.end());
```

## Benchmark Results

Configuration: chain length 1-1, 10 hardware threads, 8-thread parallel runs.

| Function | Mode | Pool | Unique MRs | Time (ms) |
|----------|------|------|-----------|-----------|
| sin | Seq | base (28) | 9 | 5.7 |
| sin | Par/8 | base | 9 | 3.1 |
| sin | Seq | +FC (76) | 9 | 22.1 |
| sin | Par/8 | +FC | 9 | 7.7 |
| cos | Seq | base | 9 | 3.6 |
| cos | Par/8 | base | 9 | 2.4 |
| cos | Seq | +FC | 9 | 14.8 |
| cos | Par/8 | +FC | 9 | 8.1 |
| tan | Seq | base | 13 | 5.2 |
| tan | Par/8 | base | 13 | 2.4 |
| tan | Seq | +FC | **18** | 19.5 |
| tan | Par/8 | +FC | **18** | 9.0 |
| asin | Seq | base | 324 | 20.5 |
| asin | Par/8 | base | 324 | 7.4 |
| asin | Seq | +FC | **2613** | 109.1 |

## Observations

1. **FC finds more MRs for tan** — 18 vs 13 unique MRs (38% more). Fork-combine discovers compound relationships like `Add(Double(x), Triple(x)) == Mul(x, 5)` that sequential chains cannot express.

2. **Combinatorial cost is real** — the pool grows from 28 to 76 (+48 FC transformers), and search time increases ~3-5x. With chain length 2, output combos go from 28^2 = 784 to 76^2 = 5776.

3. **Parallelization helps more with FC** — larger pools benefit more from threading. `sin` +FC sees **2.86x** speedup at 8 threads vs **1.80x** without FC.

4. **asin +FC explodes** — 2613 unique MRs vs 324, 109ms vs 20.5ms. This highlights that curated primitive subsets are essential to keep search tractable.

5. **Design is opt-in** — zero engine changes required. Users explicitly build FC pools and add them to their transformer vectors. The `MREngine` treats `ForkCombineTransformer` like any other `ITransformer`.

## Files

- `aya_v2.hpp` (lines 288-383) — implementation
- `aya_v2_tests.cpp` (sections 15-18) — unit tests and timing
- `aya_v2_parallel_benchmark.cpp` — parallel benchmark with FC pools
