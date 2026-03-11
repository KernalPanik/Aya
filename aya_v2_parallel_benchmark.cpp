/**
 * aya_v2_parallel_bench.cpp — Sequential vs Parallel MR Search benchmark.
 *
 * Covers all test functions from aya_v2_tests.cpp as benchmarks:
 *   - Trig functions (sin, cos, tan, asin, acos, atan, sin², cos²)
 *   - exp / log / sqrt
 *   - pow(base, exp)
 *   - vector<int> push/pop
 *   - 2D rotation matrices
 *   - std::string transforms
 *   - Point struct (distance invariance)
 *   - std::set operations
 *   - std::map transforms
 *   - Heap property preservation
 *
 * Build: g++ -std=c++20 -O2 -pthread -o aya_par_bench aya_v2_parallel_benchmark.cpp
 */

#include "aya_v2_parallel.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <vector>

constexpr double PI = 3.14159265358979323846;

inline std::function<bool(double, double)> Eq = [](double a, double b) {
    return std::abs(a - b) < 1e-6 || (std::isnan(a) && std::isnan(b));
};

std::vector<std::any> GenDoubles(size_t n, double lo, double hi) {
    static std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(lo, hi);
    std::vector<std::any> out;
    for (size_t i = 0; i < n; ++i) out.push_back(dist(rng));
    return out;
}

// ===========================================================================
// Double transformer factories (shared by trig, exp/log/sqrt)
// ===========================================================================

std::vector<Aya::TransformerPtr> MakeNoArgTf() {
    using F = std::function<void(double&)>;
    return Aya::MakeTransformers<double>(
        std::vector<F>{
            [](double& x) { x = std::cos(x * PI / 180.0); },
            [](double& x) { x = std::sin(x * PI / 180.0); },
            [](double& x) { auto c=std::cos(x*PI/180); auto s=std::sin(x*PI/180); x=c/s; },
            [](double& x) { auto c=std::cos(x*PI/180); auto s=std::sin(x*PI/180); x=s/c; },
            [](double& x) { x = std::tan(x * PI / 180.0); },
            [](double& x) { if(x>1||x<-1) throw std::domain_error(""); x=std::asin(x)*180/PI; },
            [](double& x) { if(x>1||x<-1) throw std::domain_error(""); x=std::acos(x)*180/PI; },
            [](double& x) { x = std::atan(x) * 180 / PI; },
            [](double& x) { double s=std::sin(x*PI/180); x=s*s; },
            [](double& x) { double c=std::cos(x*PI/180); x=c*c; },
            [](double& x) { x = x * x; },
            [](double& x) { if(x<0) throw std::domain_error(""); x=std::sqrt(x); },
        },
        {"Cos","Sin","CosDivSin","SinDivCos","Tan","Asin","Acos","Atan",
         "Sin2","Cos2","Square","Root"}
    );
}

std::vector<Aya::TransformerPtr> MakeArgTf() {
    using F = std::function<void(double&, double)>;
    std::vector<Aya::TransformerPtr> out;
    struct Op { std::string name; F fn; };
    std::vector<Op> ops = {
        {"Add", [](double& x, double v) { x += v; }},
        {"Mul", [](double& x, double v) { x *= v; }},
        {"Sub", [](double& x, double v) { x -= v; }},
        {"Div", [](double& x, double v) { if(v!=0) x/=v; }},
    };
    for (auto& op : ops)
        for (double v : {-1.0, 1.0, 2.0, -2.0})
            out.push_back(Aya::MakeTransformer<double, double>(op.name, op.fn, v));
    return out;
}

std::vector<Aya::TransformerPtr> MakeAllTf() {
    auto a = MakeNoArgTf();
    auto b = MakeArgTf();
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

// Fork-combine combiners for doubles
std::vector<std::pair<std::string, std::function<double(double, double)>>> MakeDoubleFcCombiners() {
    return {
        {"Add", [](double a, double b) { return a + b; }},
        {"Sub", [](double a, double b) { return a - b; }},
        {"Mul", [](double a, double b) { return a * b; }},
        {"Div", [](double a, double b) { return b != 0 ? a / b : a; }},
    };
}

// Fork-combine pool from a curated subset of no-arg primitives
std::vector<Aya::TransformerPtr> MakeFcPool() {
    // Curated subset to keep combinatorial count manageable
    std::vector<Aya::TransformerPtr> primitives = {
        Aya::MakeTransformer<double>("Sin", std::function<void(double&)>(
            [](double& x) { x = std::sin(x * PI / 180.0); })),
        Aya::MakeTransformer<double>("Cos", std::function<void(double&)>(
            [](double& x) { x = std::cos(x * PI / 180.0); })),
        Aya::MakeTransformer<double>("Square", std::function<void(double&)>(
            [](double& x) { x = x * x; })),
        Aya::MakeTransformer<double>("Root", std::function<void(double&)>(
            [](double& x) { if(x >= 0) x = std::sqrt(x); })),
    };
    return Aya::MakeForkCombinePool<double>(primitives, MakeDoubleFcCombiners());
}

std::vector<Aya::TransformerPtr> MakeAllTfWithFc() {
    auto all = MakeAllTf();
    auto fc = MakeFcPool();
    all.insert(all.end(), fc.begin(), fc.end());
    return all;
}

// ===========================================================================
// Generic bench harness
// ===========================================================================

void PrintBenchHeader(const char* name, size_t ic, size_t oc, unsigned hw) {
    std::cout << "\n" << name << " (config " << ic << "-" << oc
              << ", hardware_concurrency=" << hw << ")\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left
              << std::setw(12) << "Mode"
              << std::setw(10) << "Threads"
              << std::setw(12) << "Raw MRs"
              << std::setw(14) << "Unique MRs"
              << std::setw(14) << "Time (ms)"
              << std::setw(12) << "Speedup"
              << "\n";
    std::cout << std::string(80, '-') << "\n";
}

void PrintBenchRow(const char* mode, size_t threads, size_t raw, size_t uniq,
                   double ms, double seq_ms) {
    std::ostringstream speedup;
    speedup << std::fixed << std::setprecision(2) << (seq_ms / ms) << "x";

    std::cout << std::setw(12) << mode
              << std::setw(10) << threads
              << std::setw(12) << raw
              << std::setw(14) << uniq
              << std::setw(14) << std::fixed << std::setprecision(1) << ms
              << std::setw(12) << (seq_ms == ms ? "1.00x" : speedup.str())
              << "\n";
}

std::vector<size_t> GetThreadCounts() {
    unsigned hw = std::thread::hardware_concurrency();
    std::vector<size_t> tc = {1, 2};
    if (hw >= 4)  tc.push_back(4);
    if (hw >= 8)  tc.push_back(8);
    if (hw >= 16) tc.push_back(16);
    return tc;
}

// ===========================================================================
// 1. Single-arg double→double benchmark (trig, exp, log, sqrt)
// ===========================================================================

void BenchFunction(
    const char* name,
    std::function<double(double)> func,
    const std::vector<std::vector<std::any>>& sample,
    const std::vector<std::vector<std::any>>& verify,
    size_t ic, size_t oc,
    bool use_fork_combine = false
) {
    unsigned hw = std::thread::hardware_concurrency();
    auto thread_counts = GetThreadCounts();

    std::string label = std::string(name) + (use_fork_combine ? " +FC" : "");
    PrintBenchHeader(label.c_str(), ic, oc, hw);

    auto make_pool = [&]() { return use_fork_combine ? MakeAllTfWithFc() : MakeAllTf(); };

    // Sequential baseline
    size_t seq_raw, seq_uniq;
    double seq_ms;
    {
        auto all = make_pool();
        auto arg_tf = MakeArgTf();
        auto engine = Aya::MREngine<double, double>(func, Eq);
        engine.AddInputTransformers(0, all);
        engine.AddOutputTransformers(all);
        engine.AddVariableOutputTransformers(
            arg_tf, std::vector<std::vector<size_t>>(arg_tf.size(), {0}));

        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, ic, oc, 0, 0, 1.0f, true);
        auto t1 = std::chrono::high_resolution_clock::now();

        seq_raw = mrs.size();
        seq_uniq = Aya::ConsolidateMRs(mrs).size();
        seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    PrintBenchRow("Sequential", 1, seq_raw, seq_uniq, seq_ms, seq_ms);

    // Parallel with varying thread counts
    for (size_t t : thread_counts) {
        auto all = make_pool();
        auto arg_tf = MakeArgTf();
        auto engine = Aya::ParallelMREngine<double, double>(func, Eq, t);
        engine.AddInputTransformers(0, all);
        engine.AddOutputTransformers(all);
        engine.AddVariableOutputTransformers(
            arg_tf, std::vector<std::vector<size_t>>(arg_tf.size(), {0}));

        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, ic, oc, 0, 0, 1.0f, true);
        auto t1 = std::chrono::high_resolution_clock::now();

        size_t par_raw = mrs.size();
        size_t par_uniq = Aya::ConsolidateMRs(mrs).size();
        double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PrintBenchRow("Parallel", t, par_raw, par_uniq, par_ms, seq_ms);
    }
}

// ===========================================================================
// 2. pow(base, exp) benchmark
// ===========================================================================

void BenchPow() {
    unsigned hw = std::thread::hardware_concurrency();
    auto thread_counts = GetThreadCounts();
    PrintBenchHeader("pow(base,exp)", 1, 1, hw);

    auto setup_pow = [](auto& engine) {
        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
                [](double& x, double c) { x *= c; }), 1.0),
        });
        engine.AddInputTransformers(1, {
            Aya::MakeTransformer<double, double>("Add", std::function<void(double&,double)>(
                [](double& x, double c) { x += c; }), 1.0),
            Aya::MakeTransformer<double, double>("Add", std::function<void(double&,double)>(
                [](double& x, double c) { x += c; }), -1.0),
            Aya::MakeTransformer<double, double>("Add", std::function<void(double&,double)>(
                [](double& x, double c) { x += c; }), 2.0),
            Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
                [](double& x, double c) { x *= c; }), 2.0),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<double>("Square", std::function<void(double&)>(
                [](double& x) { x = x * x; })),
        });
        engine.AddVariableOutputTransformers(
            {
                Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
                    [](double& x, double c) { x *= c; }), 0.0),
                Aya::MakeTransformer<double, double>("Div", std::function<void(double&,double)>(
                    [](double& x, double c) { if(c!=0) x /= c; }), 0.0),
            },
            {{0}, {0}}
        );
    };

    std::vector<std::vector<std::any>> sample = {
        {std::any(2.0), std::any(3.0), std::any(5.0)},
        {std::any(2.0), std::any(3.0), std::any(4.0)}
    };
    std::vector<std::vector<std::any>> verify = {
        {std::any(2.0), std::any(4.0), std::any(7.0)},
        {std::any(2.0), std::any(5.0), std::any(3.0)}
    };

    auto pow_func = [](double b, double e) -> double { return std::pow(b, e); };

    // Sequential
    size_t seq_raw, seq_uniq;
    double seq_ms;
    {
        auto engine = Aya::MREngine<double, double, double>(pow_func, Eq);
        setup_pow(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f, true);
        auto t1 = std::chrono::high_resolution_clock::now();
        seq_raw = mrs.size();
        seq_uniq = Aya::ConsolidateMRs(mrs).size();
        seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    PrintBenchRow("Sequential", 1, seq_raw, seq_uniq, seq_ms, seq_ms);

    for (size_t t : thread_counts) {
        auto engine = Aya::ParallelMREngine<double, double, double>(pow_func, Eq, t);
        setup_pow(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f, true);
        auto t1 = std::chrono::high_resolution_clock::now();
        size_t par_raw = mrs.size();
        size_t par_uniq = Aya::ConsolidateMRs(mrs).size();
        double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PrintBenchRow("Parallel", t, par_raw, par_uniq, par_ms, seq_ms);
    }
}

// ===========================================================================
// 3. vector<int> push/pop benchmark
// ===========================================================================

void BenchVectorPushPop() {
    using Vec = std::vector<int>;
    unsigned hw = std::thread::hardware_concurrency();
    auto thread_counts = GetThreadCounts();
    PrintBenchHeader("vector<int> push/pop", 1, 1, hw);

    auto vec_eq = [](Vec a, Vec b) { return a == b; };
    auto vec_func = [](Vec v) -> Vec { return v; };

    auto setup_vec = [](auto& engine) {
        using Vec = std::vector<int>;
        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<Vec>("PushBack_1", std::function<void(Vec&)>(
                [](Vec& v) { v.push_back(1); })),
            Aya::MakeTransformer<Vec>("PushBack_2", std::function<void(Vec&)>(
                [](Vec& v) { v.push_back(2); })),
            Aya::MakeTransformer<Vec>("PopBack", std::function<void(Vec&)>(
                [](Vec& v) { if (!v.empty()) v.pop_back(); })),
            Aya::MakeTransformer<Vec>("Reverse", std::function<void(Vec&)>(
                [](Vec& v) { std::reverse(v.begin(), v.end()); })),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<Vec>("PushBack_1", std::function<void(Vec&)>(
                [](Vec& v) { v.push_back(1); })),
            Aya::MakeTransformer<Vec>("PushBack_2", std::function<void(Vec&)>(
                [](Vec& v) { v.push_back(2); })),
            Aya::MakeTransformer<Vec>("PopBack", std::function<void(Vec&)>(
                [](Vec& v) { if (!v.empty()) v.pop_back(); })),
            Aya::MakeTransformer<Vec>("Reverse", std::function<void(Vec&)>(
                [](Vec& v) { std::reverse(v.begin(), v.end()); })),
        });
    };

    std::vector<std::vector<std::any>> sample = {
        {std::any(Vec{1,2,3}), std::any(Vec{}), std::any(Vec{0})}
    };
    std::vector<std::vector<std::any>> verify = {
        {std::any(Vec{1,2,3,4}), std::any(Vec{5,6}), std::any(Vec{923231,111142,123}), std::any(Vec{1})}
    };

    size_t seq_raw, seq_uniq;
    double seq_ms;
    {
        auto engine = Aya::MREngine<Vec, Vec>(vec_func, vec_eq);
        setup_vec(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        seq_raw = mrs.size();
        seq_uniq = Aya::ConsolidateMRs(mrs).size();
        seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    PrintBenchRow("Sequential", 1, seq_raw, seq_uniq, seq_ms, seq_ms);

    for (size_t t : thread_counts) {
        auto engine = Aya::ParallelMREngine<Vec, Vec>(vec_func, vec_eq, t);
        setup_vec(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        size_t par_raw = mrs.size();
        size_t par_uniq = Aya::ConsolidateMRs(mrs).size();
        double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PrintBenchRow("Parallel", t, par_raw, par_uniq, par_ms, seq_ms);
    }
}

// ===========================================================================
// 4. 2D Rotation benchmark
// ===========================================================================

using Vec2 = std::array<double, 2>;

inline void Rotate(Vec2& v, double deg) {
    double rad = deg * PI / 180.0;
    double c = std::cos(rad), s = std::sin(rad);
    double x = v[0] * c - v[1] * s;
    double y = v[0] * s + v[1] * c;
    v[0] = x; v[1] = y;
}

void BenchRotation() {
    unsigned hw = std::thread::hardware_concurrency();
    auto thread_counts = GetThreadCounts();
    PrintBenchHeader("2D rotation", 5, 2, hw);

    auto vec2_eq = [](Vec2 a, Vec2 b) {
        return std::abs(a[0]-b[0]) < 1e-6 && std::abs(a[1]-b[1]) < 1e-6;
    };
    auto rot_func = [](Vec2 v) -> Vec2 { return v; };

    auto setup_rot = [](auto& engine) {
        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<Vec2>("Rot15",  std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 15); })),
            Aya::MakeTransformer<Vec2>("Rot30",  std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 30); })),
            Aya::MakeTransformer<Vec2>("Rot60",  std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 60); })),
            Aya::MakeTransformer<Vec2>("Rot90",  std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 90); })),
            Aya::MakeTransformer<Vec2>("Rot120", std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 120); })),
            Aya::MakeTransformer<Vec2>("Rot150", std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 150); })),
            Aya::MakeTransformer<Vec2>("RotN15", std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, -15); })),
            Aya::MakeTransformer<Vec2>("RotN30", std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, -30); })),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<Vec2>("Rot15",  std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 15); })),
            Aya::MakeTransformer<Vec2>("Rot30",  std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 30); })),
            Aya::MakeTransformer<Vec2>("Rot60",  std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 60); })),
            Aya::MakeTransformer<Vec2>("Rot90",  std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 90); })),
            Aya::MakeTransformer<Vec2>("Rot120", std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 120); })),
            Aya::MakeTransformer<Vec2>("Rot150", std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 150); })),
            Aya::MakeTransformer<Vec2>("RotN15", std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, -15); })),
            Aya::MakeTransformer<Vec2>("RotN30", std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, -30); })),
        });
    };

    std::mt19937_64 rng(123);
    std::uniform_real_distribution<double> dist(0, 60);
    std::vector<std::any> vecs;
    for (int i = 0; i < 5; ++i) vecs.push_back(Vec2{dist(rng), dist(rng)});
    std::vector<std::vector<std::any>> data = {vecs};

    size_t seq_raw, seq_uniq;
    double seq_ms;
    {
        auto engine = Aya::MREngine<Vec2, Vec2>(rot_func, vec2_eq);
        setup_rot(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(data, data, 5, 2, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        seq_raw = mrs.size();
        seq_uniq = Aya::ConsolidateMRs(mrs).size();
        seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    PrintBenchRow("Sequential", 1, seq_raw, seq_uniq, seq_ms, seq_ms);

    for (size_t t : thread_counts) {
        auto engine = Aya::ParallelMREngine<Vec2, Vec2>(rot_func, vec2_eq, t);
        setup_rot(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(data, data, 5, 2, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        size_t par_raw = mrs.size();
        size_t par_uniq = Aya::ConsolidateMRs(mrs).size();
        double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PrintBenchRow("Parallel", t, par_raw, par_uniq, par_ms, seq_ms);
    }
}

// ===========================================================================
// 5. std::string transforms benchmark
// ===========================================================================

void BenchString() {
    using S = std::string;
    unsigned hw = std::thread::hardware_concurrency();
    auto thread_counts = GetThreadCounts();
    PrintBenchHeader("string sort", 1, 1, hw);

    auto str_eq = [](S a, S b) { return a == b; };
    auto str_func = [](S s) -> S { std::sort(s.begin(), s.end()); return s; };

    auto setup_str = [](auto& engine) {
        using S = std::string;
        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<S>("Reverse", std::function<void(S&)>(
                [](S& s) { std::reverse(s.begin(), s.end()); })),
            Aya::MakeTransformer<S>("RotateL1", std::function<void(S&)>(
                [](S& s) { if(!s.empty()) std::rotate(s.begin(), s.begin()+1, s.end()); })),
            Aya::MakeTransformer<S>("Duplicate", std::function<void(S&)>(
                [](S& s) { s += s; })),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<S>("Identity", std::function<void(S&)>(
                [](S& s) { })),
            Aya::MakeTransformer<S>("Reverse", std::function<void(S&)>(
                [](S& s) { std::reverse(s.begin(), s.end()); })),
            Aya::MakeTransformer<S>("Duplicate", std::function<void(S&)>(
                [](S& s) { s += s; })),
        });
    };

    std::vector<std::vector<std::any>> sample = {
        {std::any(S("hello")), std::any(S("world")), std::any(S("abc")), std::any(S("zzz"))}
    };
    std::vector<std::vector<std::any>> verify = {
        {std::any(S("testing")), std::any(S("metamorphic")), std::any(S("xyz"))}
    };

    size_t seq_raw, seq_uniq;
    double seq_ms;
    {
        auto engine = Aya::MREngine<S, S>(str_func, str_eq);
        setup_str(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        seq_raw = mrs.size();
        seq_uniq = Aya::ConsolidateMRs(mrs).size();
        seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    PrintBenchRow("Sequential", 1, seq_raw, seq_uniq, seq_ms, seq_ms);

    for (size_t t : thread_counts) {
        auto engine = Aya::ParallelMREngine<S, S>(str_func, str_eq, t);
        setup_str(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        size_t par_raw = mrs.size();
        size_t par_uniq = Aya::ConsolidateMRs(mrs).size();
        double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PrintBenchRow("Parallel", t, par_raw, par_uniq, par_ms, seq_ms);
    }
}

// ===========================================================================
// 6. Point struct (distance invariance) benchmark
// ===========================================================================

struct Point {
    double x, y;
    bool operator==(const Point& o) const {
        return std::abs(x - o.x) < 1e-6 && std::abs(y - o.y) < 1e-6;
    }
};

void BenchPoint() {
    unsigned hw = std::thread::hardware_concurrency();
    auto thread_counts = GetThreadCounts();
    PrintBenchHeader("Point distance", 1, 1, hw);

    auto point_func = [](Point p) -> double { return std::sqrt(p.x*p.x + p.y*p.y); };

    auto setup_point = [](auto& engine) {
        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<Point>("NegateX", std::function<void(Point&)>(
                [](Point& p) { p.x = -p.x; })),
            Aya::MakeTransformer<Point>("NegateY", std::function<void(Point&)>(
                [](Point& p) { p.y = -p.y; })),
            Aya::MakeTransformer<Point>("NegateXY", std::function<void(Point&)>(
                [](Point& p) { p.x = -p.x; p.y = -p.y; })),
            Aya::MakeTransformer<Point>("SwapXY", std::function<void(Point&)>(
                [](Point& p) { std::swap(p.x, p.y); })),
            Aya::MakeTransformer<Point>("Rotate90", std::function<void(Point&)>(
                [](Point& p) { double tmp = p.x; p.x = -p.y; p.y = tmp; })),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<double>("Identity", std::function<void(double&)>(
                [](double& x) { })),
            Aya::MakeTransformer<double>("Negate", std::function<void(double&)>(
                [](double& x) { x = -x; })),
            Aya::MakeTransformer<double>("Square", std::function<void(double&)>(
                [](double& x) { x = x * x; })),
        });
    };

    std::vector<std::any> points = {
        std::any(Point{3.0, 4.0}),
        std::any(Point{1.0, 0.0}),
        std::any(Point{0.0, 5.0}),
        std::any(Point{2.0, 2.0}),
        std::any(Point{-1.0, 3.0}),
    };
    std::vector<std::vector<std::any>> data = {points};

    size_t seq_raw, seq_uniq;
    double seq_ms;
    {
        auto engine = Aya::MREngine<double, Point>(point_func, Eq);
        setup_point(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(data, data, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        seq_raw = mrs.size();
        seq_uniq = Aya::ConsolidateMRs(mrs).size();
        seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    PrintBenchRow("Sequential", 1, seq_raw, seq_uniq, seq_ms, seq_ms);

    for (size_t t : thread_counts) {
        auto engine = Aya::ParallelMREngine<double, Point>(point_func, Eq, t);
        setup_point(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(data, data, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        size_t par_raw = mrs.size();
        size_t par_uniq = Aya::ConsolidateMRs(mrs).size();
        double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PrintBenchRow("Parallel", t, par_raw, par_uniq, par_ms, seq_ms);
    }
}

// ===========================================================================
// 7. std::set benchmark
// ===========================================================================

void BenchSet() {
    using S = std::set<int>;
    unsigned hw = std::thread::hardware_concurrency();
    auto thread_counts = GetThreadCounts();
    PrintBenchHeader("set<int> ops", 1, 1, hw);

    auto set_eq = [](S a, S b) { return a == b; };
    auto set_func = [](S s) -> S { return s; };

    auto setup_set = [](auto& engine) {
        using S = std::set<int>;
        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<S>("AddElem_0", std::function<void(S&)>(
                [](S& s) { s.insert(0); })),
            Aya::MakeTransformer<S>("AddElem_99", std::function<void(S&)>(
                [](S& s) { s.insert(99); })),
            Aya::MakeTransformer<S>("RemoveMin", std::function<void(S&)>(
                [](S& s) { if(!s.empty()) s.erase(s.begin()); })),
            Aya::MakeTransformer<S>("RemoveMax", std::function<void(S&)>(
                [](S& s) { if(!s.empty()) s.erase(std::prev(s.end())); })),
            Aya::MakeTransformer<S>("UnionSelf", std::function<void(S&)>(
                [](S& s) { })),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<S>("AddElem_0", std::function<void(S&)>(
                [](S& s) { s.insert(0); })),
            Aya::MakeTransformer<S>("AddElem_99", std::function<void(S&)>(
                [](S& s) { s.insert(99); })),
            Aya::MakeTransformer<S>("RemoveMin", std::function<void(S&)>(
                [](S& s) { if(!s.empty()) s.erase(s.begin()); })),
            Aya::MakeTransformer<S>("RemoveMax", std::function<void(S&)>(
                [](S& s) { if(!s.empty()) s.erase(std::prev(s.end())); })),
        });
    };

    std::vector<std::vector<std::any>> sample = {
        {std::any(S{1,2,3}), std::any(S{5,10,15,20}), std::any(S{})}
    };
    std::vector<std::vector<std::any>> verify = {
        {std::any(S{1,2,3,4,5}), std::any(S{100}), std::any(S{-1,0,1})}
    };

    size_t seq_raw, seq_uniq;
    double seq_ms;
    {
        auto engine = Aya::MREngine<S, S>(set_func, set_eq);
        setup_set(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        seq_raw = mrs.size();
        seq_uniq = Aya::ConsolidateMRs(mrs).size();
        seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    PrintBenchRow("Sequential", 1, seq_raw, seq_uniq, seq_ms, seq_ms);

    for (size_t t : thread_counts) {
        auto engine = Aya::ParallelMREngine<S, S>(set_func, set_eq, t);
        setup_set(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        size_t par_raw = mrs.size();
        size_t par_uniq = Aya::ConsolidateMRs(mrs).size();
        double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PrintBenchRow("Parallel", t, par_raw, par_uniq, par_ms, seq_ms);
    }
}

// ===========================================================================
// 8. std::map benchmark
// ===========================================================================

void BenchMap() {
    using M = std::map<std::string, int>;
    unsigned hw = std::thread::hardware_concurrency();
    auto thread_counts = GetThreadCounts();
    PrintBenchHeader("map<string,int> ops", 1, 1, hw);

    auto map_eq = [](M a, M b) { return a == b; };
    auto map_func = [](M m) -> M { return m; };

    auto setup_map = [](auto& engine) {
        using M = std::map<std::string, int>;
        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<M>("InsertA_1", std::function<void(M&)>(
                [](M& m) { m["a"] = 1; })),
            Aya::MakeTransformer<M>("InsertB_2", std::function<void(M&)>(
                [](M& m) { m["b"] = 2; })),
            Aya::MakeTransformer<M>("EraseA", std::function<void(M&)>(
                [](M& m) { m.erase("a"); })),
            Aya::MakeTransformer<M>("EraseB", std::function<void(M&)>(
                [](M& m) { m.erase("b"); })),
            Aya::MakeTransformer<M>("Clear", std::function<void(M&)>(
                [](M& m) { m.clear(); })),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<M>("InsertA_1", std::function<void(M&)>(
                [](M& m) { m["a"] = 1; })),
            Aya::MakeTransformer<M>("InsertB_2", std::function<void(M&)>(
                [](M& m) { m["b"] = 2; })),
            Aya::MakeTransformer<M>("EraseA", std::function<void(M&)>(
                [](M& m) { m.erase("a"); })),
            Aya::MakeTransformer<M>("EraseB", std::function<void(M&)>(
                [](M& m) { m.erase("b"); })),
            Aya::MakeTransformer<M>("Clear", std::function<void(M&)>(
                [](M& m) { m.clear(); })),
        });
    };

    M m1 = {{"a", 1}, {"b", 2}};
    M m2 = {{"x", 10}};
    M m3 = {};
    M m4 = {{"a", 1}, {"b", 2}, {"c", 3}};

    std::vector<std::vector<std::any>> sample = {
        {std::any(m1), std::any(m2), std::any(m3)}
    };
    std::vector<std::vector<std::any>> verify = {
        {std::any(m1), std::any(m4), std::any(M{{"z",99}})}
    };

    size_t seq_raw, seq_uniq;
    double seq_ms;
    {
        auto engine = Aya::MREngine<M, M>(map_func, map_eq);
        setup_map(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        seq_raw = mrs.size();
        seq_uniq = Aya::ConsolidateMRs(mrs).size();
        seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    PrintBenchRow("Sequential", 1, seq_raw, seq_uniq, seq_ms, seq_ms);

    for (size_t t : thread_counts) {
        auto engine = Aya::ParallelMREngine<M, M>(map_func, map_eq, t);
        setup_map(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        size_t par_raw = mrs.size();
        size_t par_uniq = Aya::ConsolidateMRs(mrs).size();
        double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PrintBenchRow("Parallel", t, par_raw, par_uniq, par_ms, seq_ms);
    }
}

// ===========================================================================
// 9. Heap property benchmark
// ===========================================================================

void BenchHeap() {
    using Vec = std::vector<int>;
    unsigned hw = std::thread::hardware_concurrency();
    auto thread_counts = GetThreadCounts();
    PrintBenchHeader("heap property", 1, 1, hw);

    auto heap_eq = [](Vec a, Vec b) -> bool {
        if (a.empty() && b.empty()) return true;
        if (a.empty() || b.empty()) return false;
        return a.front() == b.front();
    };
    auto heap_func = [](Vec v) -> Vec { std::make_heap(v.begin(), v.end()); return v; };

    auto setup_heap = [](auto& engine) {
        using Vec = std::vector<int>;
        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<Vec>("Reverse", std::function<void(Vec&)>(
                [](Vec& v) { std::reverse(v.begin(), v.end()); })),
            Aya::MakeTransformer<Vec>("Sort", std::function<void(Vec&)>(
                [](Vec& v) { std::sort(v.begin(), v.end()); })),
            Aya::MakeTransformer<Vec>("Shuffle", std::function<void(Vec&)>(
                [](Vec& v) {
                    static std::mt19937 rng(42);
                    std::shuffle(v.begin(), v.end(), rng);
                })),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<Vec>("Sort", std::function<void(Vec&)>(
                [](Vec& v) { std::sort(v.begin(), v.end()); })),
            Aya::MakeTransformer<Vec>("SortDesc", std::function<void(Vec&)>(
                [](Vec& v) { std::sort(v.begin(), v.end(), std::greater<>()); })),
        });
    };

    std::vector<std::vector<std::any>> sample = {
        {std::any(Vec{5,3,8,1,2}), std::any(Vec{10,20,30}), std::any(Vec{1})}
    };
    std::vector<std::vector<std::any>> verify = {
        {std::any(Vec{7,3,9,1,5,2}), std::any(Vec{100,50,75,25})}
    };

    size_t seq_raw, seq_uniq;
    double seq_ms;
    {
        auto engine = Aya::MREngine<Vec, Vec>(heap_func, heap_eq);
        setup_heap(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        seq_raw = mrs.size();
        seq_uniq = Aya::ConsolidateMRs(mrs).size();
        seq_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }
    PrintBenchRow("Sequential", 1, seq_raw, seq_uniq, seq_ms, seq_ms);

    for (size_t t : thread_counts) {
        auto engine = Aya::ParallelMREngine<Vec, Vec>(heap_func, heap_eq, t);
        setup_heap(engine);
        auto t0 = std::chrono::high_resolution_clock::now();
        auto mrs = engine.Search(sample, verify, 1, 1, 0, 0, 1.0f);
        auto t1 = std::chrono::high_resolution_clock::now();
        size_t par_raw = mrs.size();
        size_t par_uniq = Aya::ConsolidateMRs(mrs).size();
        double par_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        PrintBenchRow("Parallel", t, par_raw, par_uniq, par_ms, seq_ms);
    }
}

// ===========================================================================
// Main
// ===========================================================================

int main(int argc, char* argv[]) {
    size_t ic = 1, oc = 1;
    if (argc >= 3) {
        ic = static_cast<size_t>(std::atoi(argv[1]));
        oc = static_cast<size_t>(std::atoi(argv[2]));
    }
    // Clamp to 2-2 max
    if (ic > 2) ic = 2;
    if (oc > 2) oc = 2;

    auto trig_s = {GenDoubles(10, 30, 90)};
    auto trig_v = {GenDoubles(10, 30, 90)};
    auto small_s = {GenDoubles(10, 0, 0)};
    auto small_v = {GenDoubles(10, 0, 0)};
    auto pos_s = {GenDoubles(10, 1, 5)};
    auto pos_v = {GenDoubles(10, 1, 5)};

    size_t fc_count = MakeFcPool().size();
    size_t base_count = MakeAllTf().size();

    std::cout << std::string(80, '=') << "\n"
              << "Sequential vs Parallel MR Search Benchmark (config "
              << ic << "-" << oc << ", max 2-2)\n"
              << "Base pool: " << base_count << " transformers"
              << " | +FC pool: " << base_count + fc_count
              << " (" << fc_count << " fork-combine)"
              << " | Inputs: 10 sample, 10 verify\n"
              << std::string(80, '=') << "\n";

    // Bench each function without and with fork-combine
    struct FuncEntry {
        const char* name;
        std::function<double(double)> func;
        const std::vector<std::vector<std::any>>* sample;
        const std::vector<std::vector<std::any>>* verify;
    };

    std::vector<std::vector<std::any>> trig_s_v(trig_s);
    std::vector<std::vector<std::any>> trig_v_v(trig_v);
    std::vector<std::vector<std::any>> small_s_v(small_s);
    std::vector<std::vector<std::any>> small_v_v(small_v);
    std::vector<std::vector<std::any>> pos_s_v(pos_s);
    std::vector<std::vector<std::any>> pos_v_v(pos_v);

    std::vector<FuncEntry> funcs = {
        {"sin",  [](double x) -> double { return std::sin(x * PI / 180.0); }, &trig_s_v, &trig_v_v},
        {"cos",  [](double x) -> double { return std::cos(x * PI / 180.0); }, &trig_s_v, &trig_v_v},
        {"tan",  [](double x) -> double { return std::tan(x * PI / 180.0); }, &trig_s_v, &trig_v_v},
        {"asin", [](double x) -> double {
            if(x>1||x<-1) throw std::domain_error("");
            return std::asin(x)*180/PI;
        }, &small_s_v, &small_v_v},
        {"acos", [](double x) -> double {
            if(x>1||x<-1) throw std::domain_error("");
            return std::acos(x)*180/PI;
        }, &small_s_v, &small_v_v},
        {"atan", [](double x) -> double { return std::atan(x) * 180 / PI; }, &trig_s_v, &trig_v_v},
        {"sin^2",[](double x) -> double { double s=std::sin(x*PI/180); return s*s; }, &trig_s_v, &trig_v_v},
        {"cos^2",[](double x) -> double { double c=std::cos(x*PI/180); return c*c; }, &trig_s_v, &trig_v_v},
        {"exp",  [](double x) -> double { return std::exp(x); }, &pos_s_v, &pos_v_v},
        {"log",  [](double x) -> double {
            if (x <= 0) throw std::domain_error("");
            return std::log(x);
        }, &pos_s_v, &pos_v_v},
        {"sqrt", [](double x) -> double {
            if (x < 0) throw std::domain_error("");
            return std::sqrt(x);
        }, &pos_s_v, &pos_v_v},
    };

    for (auto& f : funcs) {
        // Without fork-combine
        BenchFunction(f.name, f.func, *f.sample, *f.verify, ic, oc, false);
        // With fork-combine
        BenchFunction(f.name, f.func, *f.sample, *f.verify, ic, oc, true);
    }

    // --- Non-double benchmarks ---
    BenchPow();
    BenchVectorPushPop();
    BenchRotation();
    BenchString();
    BenchPoint();
    BenchSet();
    BenchMap();
    BenchHeap();

    std::cout << "\n" << std::string(80, '=') << "\n"
              << "Benchmark complete (config " << ic << "-" << oc << ")\n"
              << std::string(80, '=') << "\n";

    return 0;
}
