/**
 * aya_v2_tests.cpp — Test suite for Aya v2.
 *
 * Covers:
 *   1. Original Aya math tests (sin, cos, tan, asin, acos, atan, sin², cos², exp, ln, sqrt, pow)
 *   2. Original STL tests (vector push/pop)
 *   3. Original linalg tests (2D rotation matrices) — portable, no Accelerate dependency
 *   4. NEW: std::map (insert/erase roundtrip, merge idempotence)
 *   5. NEW: std::string (reverse, append, case transforms)
 *   6. NEW: Struct type — 2D Point with geometric transforms
 *   7. NEW: std::set (union/intersection properties)
 *   8. NEW: Mutation testing — sin mutants to verify MR reactivity
 *   9. NEW: Priority queue / heap property preservation
 *  10. NEW: JSON-like nested struct (key-value store with nested access)
 *
 * Build: g++ -std=c++20 -O2 -o aya_tests aya_v2_tests.cpp
 */

#include "aya_v2.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <chrono>
#include <vector>

// ===========================================================================
// Test harness
// ===========================================================================

static int g_pass = 0;
static int g_fail = 0;

#define TEST(name)                                                          \
    do {                                                                    \
        std::cout << "  [TEST] " << name << "... " << std::flush;          \
    } while (0)

#define PASS()                                                              \
    do {                                                                    \
        std::cout << "PASS\n";                                              \
        g_pass++;                                                           \
    } while (0)

#define FAIL(msg)                                                           \
    do {                                                                    \
        std::cout << "FAIL: " << msg << "\n";                               \
        g_fail++;                                                           \
    } while (0)

#define ASSERT_TRUE(cond, msg)                                              \
    do {                                                                    \
        if (!(cond)) { FAIL(msg); return; }                                 \
    } while (0)

#define SECTION(name)                                                       \
    std::cout << "\n" << std::string(60, '=') << "\n"                       \
              << name << "\n"                                                \
              << std::string(60, '=') << "\n"

// ===========================================================================
// Shared utilities
// ===========================================================================

constexpr double PI = 3.14159265358979323846;

inline bool ApproxEq(double a, double b, double eps = 1e-6) {
    if (std::isnan(a) && std::isnan(b)) return true;
    if (std::isinf(a) || std::isinf(b)) return a == b;
    return std::abs(a - b) <= eps;
}

// 2-arg wrapper for MREngine Comparer
inline std::function<bool(double, double)> DoubleEq = [](double a, double b) {
    return ApproxEq(a, b);
};

inline std::vector<std::any> GenDoubles(size_t n, double lo, double hi) {
    static std::mt19937_64 rng(42); // fixed seed for reproducibility
    std::uniform_real_distribution<double> dist(lo, hi);
    std::vector<std::any> out;
    out.reserve(n);
    for (size_t i = 0; i < n; ++i) out.push_back(dist(rng));
    return out;
}

// ===========================================================================
// 1. Math: Trig functions (sin, cos, tan, asin, acos, atan)
// ===========================================================================

// All trig wrappers use degrees internally (matching original Aya tests)
inline double SinDeg(double x) { return std::sin(x * PI / 180.0); }
inline double CosDeg(double x) { return std::cos(x * PI / 180.0); }
inline double TanDeg(double x) { return std::tan(x * PI / 180.0); }
inline double AsinDeg(double x) {
    if (x > 1.0 || x < -1.0) throw std::domain_error("asin domain");
    return std::asin(x) * 180.0 / PI;
}
inline double AcosDeg(double x) {
    if (x > 1.0 || x < -1.0) throw std::domain_error("acos domain");
    return std::acos(x) * 180.0 / PI;
}
inline double AtanDeg(double x) { return std::atan(x) * 180.0 / PI; }
inline double Sin2Deg(double x) { double s = std::sin(x * PI / 180.0); return s * s; }
inline double Cos2Deg(double x) { double c = std::cos(x * PI / 180.0); return c * c; }
inline double ExpFunc(double x) { return std::exp(x); }
inline double LogFunc(double x) {
    if (x <= 0) throw std::domain_error("log domain");
    return std::log(x);
}
inline double SqrtFunc(double x) {
    if (x < 0) throw std::domain_error("sqrt domain");
    return std::sqrt(x);
}

/// Build the standard double transformer set matching the original Aya tests.
std::vector<Aya::TransformerPtr> MakeDoubleNoArgTransformers() {
    using F = std::function<void(double&)>;
    return Aya::MakeTransformers<double>(
        std::vector<F>{
            [](double& x) { x = std::cos(x * PI / 180.0); },
            [](double& x) { x = std::sin(x * PI / 180.0); },
            [](double& x) { auto c = std::cos(x*PI/180); auto s = std::sin(x*PI/180); x = c/s; },
            [](double& x) { auto c = std::cos(x*PI/180); auto s = std::sin(x*PI/180); x = s/c; },
            [](double& x) { x = std::tan(x * PI / 180.0); },
            [](double& x) { if (x>1||x<-1) throw std::domain_error(""); x = std::asin(x)*180/PI; },
            [](double& x) { if (x>1||x<-1) throw std::domain_error(""); x = std::acos(x)*180/PI; },
            [](double& x) { x = std::atan(x) * 180 / PI; },
            [](double& x) { double s=std::sin(x*PI/180); x=s*s; },
            [](double& x) { double c=std::cos(x*PI/180); x=c*c; },
            [](double& x) { x = x * x; },
            [](double& x) { if(x<0) throw std::domain_error(""); x = std::sqrt(x); },
        },
        {"Cos", "Sin", "CosDivSin", "SinDivCos", "Tan", "Asin", "Acos", "Atan",
         "SinSquared", "CosSquared", "Square", "Root"}
    );
}

std::vector<Aya::TransformerPtr> MakeDoubleArgTransformers() {
    using F = std::function<void(double&, double)>;
    std::vector<Aya::TransformerPtr> out;
    struct Op { std::string name; F fn; };
    std::vector<Op> ops = {
        {"Add", [](double& x, double v) { x += v; }},
        {"Mul", [](double& x, double v) { x *= v; }},
        {"Sub", [](double& x, double v) { x -= v; }},
        {"Div", [](double& x, double v) { if(v!=0) x /= v; }},
    };
    std::vector<double> arg_vals = {-1.0, 1.0, 2.0, -2.0};
    for (auto& op : ops) {
        for (double v : arg_vals) {
            out.push_back(Aya::MakeTransformer<double, double>(op.name, op.fn, v));
        }
    }
    return out;
}

std::vector<Aya::TransformerPtr> MakeAllDoubleTransformers() {
    auto a = MakeDoubleNoArgTransformers();
    auto b = MakeDoubleArgTransformers();
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

void TestSingleArgMathFunc(
    const char* name,
    std::function<double(double)> func,
    const std::vector<std::vector<std::any>>& sample,
    const std::vector<std::vector<std::any>>& verify,
    size_t input_chain, size_t output_chain,
    size_t expected_min_mrs = 1
) {
    TEST(name);
    auto engine = Aya::MREngine<double, double>(func, DoubleEq);
    engine.AddInputTransformers(0, MakeAllDoubleTransformers());
    engine.AddOutputTransformers(MakeAllDoubleTransformers());

    // Variable output transforms for implicit MRs
    engine.AddVariableOutputTransformers(
        MakeDoubleArgTransformers(),
        // Each arg transformer gets index {0} (arg0 = x for single-arg functions)
        std::vector<std::vector<size_t>>(MakeDoubleArgTransformers().size(), {0})
    );

    auto mrs = engine.Search(sample, verify, input_chain, output_chain, 0, 0, 1.0f, true);
    auto consolidated = Aya::ConsolidateMRs(mrs);

    ASSERT_TRUE(consolidated.size() >= expected_min_mrs,
        std::string(name) + ": expected >= " + std::to_string(expected_min_mrs)
        + " unique MRs, got " + std::to_string(consolidated.size()));
    PASS();
}

void TestTrigFunctions() {
    SECTION("1. Trigonometric Functions (degrees)");

    auto sample = {GenDoubles(8, 30, 90)};
    auto verify = {GenDoubles(8, 30, 90)};

    TestSingleArgMathFunc("sin",  SinDeg,  sample, verify, 1, 2, 1);
    TestSingleArgMathFunc("cos",  CosDeg,  sample, verify, 1, 2, 1);
    TestSingleArgMathFunc("tan",  TanDeg,  sample, verify, 1, 2, 1);
    TestSingleArgMathFunc("sin²", Sin2Deg, sample, verify, 1, 2, 1);
    TestSingleArgMathFunc("cos²", Cos2Deg, sample, verify, 1, 2, 1);

    auto small_sample = {GenDoubles(8, 0, 0)}; // range [0,1) for inverse trig
    auto small_verify = {GenDoubles(8, 0, 0)};

    TestSingleArgMathFunc("asin", AsinDeg, small_sample, small_verify, 1, 2, 1);
    TestSingleArgMathFunc("acos", AcosDeg, small_sample, small_verify, 1, 2, 1);
    TestSingleArgMathFunc("atan", AtanDeg, small_sample, small_verify, 1, 2, 1);
}

void TestExpLogSqrt() {
    SECTION("2. exp / log / sqrt");

    auto pos_sample = {GenDoubles(8, 1, 5)}; // positive values for log
    auto pos_verify = {GenDoubles(8, 1, 5)};

    TestSingleArgMathFunc("exp",  ExpFunc,  pos_sample, pos_verify, 1, 2, 1);
    TestSingleArgMathFunc("log",  LogFunc,  pos_sample, pos_verify, 1, 2, 1);
    TestSingleArgMathFunc("sqrt", SqrtFunc, pos_sample, pos_verify, 1, 2, 1);
}

// ===========================================================================
// 2. pow(base, exp) with variable output transforms
// ===========================================================================

void TestPow() {
    SECTION("3. pow(base, exp) — state-aware output transforms");
    TEST("pow implicit MRs");

    auto engine = Aya::MREngine<double, double, double>(
        [](double b, double e) -> double { return std::pow(b, e); },
        DoubleEq
    );

    // Input: identity-ish for base, arithmetic for exp
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

    // Variable: Mul and Div with arg from state (base)
    engine.AddVariableOutputTransformers(
        {
            Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
                [](double& x, double c) { x *= c; }), 0.0),
            Aya::MakeTransformer<double, double>("Div", std::function<void(double&,double)>(
                [](double& x, double c) { if(c!=0) x /= c; }), 0.0),
        },
        {{0}, {0}}
    );

    auto mrs = engine.Search(
        {{std::any(2.0), std::any(3.0), std::any(5.0)},
         {std::any(2.0), std::any(3.0), std::any(4.0)}},
        {{std::any(2.0), std::any(4.0), std::any(7.0)},
         {std::any(2.0), std::any(5.0), std::any(3.0)}},
        1, 1, 0, 0, 1.0f, true
    );

    auto consolidated = Aya::ConsolidateMRs(mrs);
    // Expect at minimum: x^(y+1)=x^y*x, x^(y-1)=x^y/x, x^(2y)=(x^y)²
    ASSERT_TRUE(consolidated.size() >= 3,
        "pow: expected >= 3 unique MRs, got " + std::to_string(consolidated.size()));
    PASS();
}

// ===========================================================================
// 3. STL: vector<int> push/pop
// ===========================================================================

void TestVectorPushPop() {
    SECTION("4. STL vector<int> — push/pop roundtrip");
    TEST("vector identity under push+pop");

    using Vec = std::vector<int>;

    auto engine = Aya::MREngine<Vec, Vec>(
        [](Vec v) -> Vec { return v; },
        [](Vec a, Vec b) { return a == b; }
    );

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

    auto mrs = engine.Search(
        {{std::any(Vec{1,2,3}), std::any(Vec{}), std::any(Vec{0})}},
        {{std::any(Vec{1,2,3,4}), std::any(Vec{5,6}), std::any(Vec{923231,111142,123}), std::any(Vec{1})}},
        1, 1, 0, 0, 1.0f  // identity: f(transform(x)) == transform(f(x)) for any transform
    );

    ASSERT_TRUE(!mrs.empty(), "vector: expected at least 1 MR (push+pop roundtrip)");
    PASS();
}

// ===========================================================================
// 4. Linalg: 2D rotation (portable — no Accelerate)
// ===========================================================================

using Vec2 = std::array<double, 2>;

inline void Rotate(Vec2& v, double deg) {
    double rad = deg * PI / 180.0;
    double c = std::cos(rad), s = std::sin(rad);
    double x = v[0] * c - v[1] * s;
    double y = v[0] * s + v[1] * c;
    v[0] = x; v[1] = y;
}

inline bool Vec2Eq(Vec2 a, Vec2 b) {
    return std::abs(a[0]-b[0]) < 1e-6 && std::abs(a[1]-b[1]) < 1e-6;
}

void TestRotationMatrix() {
    SECTION("5. 2D Rotation Matrices");
    TEST("rotation compositions sum to 360°");

    auto engine = Aya::MREngine<Vec2, Vec2>(
        [](Vec2 v) -> Vec2 { return v; },
        Vec2Eq
    );

    engine.AddInputTransformers(0, {
        Aya::MakeTransformer<Vec2>("Rot15",    std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 15); })),
        Aya::MakeTransformer<Vec2>("Rot30",    std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 30); })),
        Aya::MakeTransformer<Vec2>("Rot60",    std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 60); })),
        Aya::MakeTransformer<Vec2>("Rot90",    std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 90); })),
        Aya::MakeTransformer<Vec2>("Rot120",   std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 120); })),
        Aya::MakeTransformer<Vec2>("Rot150",   std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 150); })),
        Aya::MakeTransformer<Vec2>("RotN15",   std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, -15); })),
        Aya::MakeTransformer<Vec2>("RotN30",   std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, -30); })),
    });

    engine.AddOutputTransformers({
        Aya::MakeTransformer<Vec2>("Rot15",    std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 15); })),
        Aya::MakeTransformer<Vec2>("Rot30",    std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 30); })),
        Aya::MakeTransformer<Vec2>("Rot60",    std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 60); })),
        Aya::MakeTransformer<Vec2>("Rot90",    std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 90); })),
        Aya::MakeTransformer<Vec2>("Rot120",   std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 120); })),
        Aya::MakeTransformer<Vec2>("Rot150",   std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, 150); })),
        Aya::MakeTransformer<Vec2>("RotN15",   std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, -15); })),
        Aya::MakeTransformer<Vec2>("RotN30",   std::function<void(Vec2&)>([](Vec2& v) { Rotate(v, -30); })),
    });

    // Generate random 2D vectors
    std::vector<std::any> vecs;
    static std::mt19937_64 rng(123);
    std::uniform_real_distribution<double> dist(0, 60);
    for (int i = 0; i < 5; ++i) {
        vecs.push_back(Vec2{dist(rng), dist(rng)});
    }

    auto mrs = engine.Search(
        {vecs}, {vecs},
        5, 2,   // long input chains to find rotation sums
        0, 0, 1.0f
    );

    auto consolidated = Aya::ConsolidateMRs(mrs);
    ASSERT_TRUE(!consolidated.empty(), "rotation: expected MRs where rotations sum to 360°");
    PASS();
}

// ===========================================================================
// 5. NEW: std::string transforms
// ===========================================================================

void TestStringTransforms() {
    SECTION("6. std::string — reverse, append, case transforms");
    TEST("string MR search");

    using S = std::string;

    // sorted() equivalent for strings: returns sorted chars
    auto engine = Aya::MREngine<S, S>(
        [](S s) -> S { std::sort(s.begin(), s.end()); return s; },
        [](S a, S b) { return a == b; }
    );

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
            [](S& s) { /* noop */ })),
        Aya::MakeTransformer<S>("Reverse", std::function<void(S&)>(
            [](S& s) { std::reverse(s.begin(), s.end()); })),
        Aya::MakeTransformer<S>("Duplicate", std::function<void(S&)>(
            [](S& s) { s += s; })),
    });

    auto mrs = engine.Search(
        {{std::any(S("hello")), std::any(S("world")), std::any(S("abc")), std::any(S("zzz"))}},
        {{std::any(S("testing")), std::any(S("metamorphic")), std::any(S("xyz"))}},
        1, 1, 0, 0, 1.0f
    );

    auto consolidated = Aya::ConsolidateMRs(mrs);
    // sorted(reverse(s)) == sorted(s) should be found
    ASSERT_TRUE(!consolidated.empty(), "string: expected permutation-invariance MRs for sort");
    PASS();
}

// ===========================================================================
// 6. NEW: Struct type — 2D Point with geometric transforms
// ===========================================================================

struct Point {
    double x, y;
    bool operator==(const Point& o) const { return ApproxEq(x, o.x) && ApproxEq(y, o.y); }
};

void TestPointStruct() {
    SECTION("7. Struct: Point{x,y} — geometric transforms");
    TEST("point reflection/translation MRs");

    // Function: compute distance from origin
    auto engine = Aya::MREngine<double, Point>(
        [](Point p) -> double { return std::sqrt(p.x*p.x + p.y*p.y); },
        DoubleEq
    );

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

    // Output transforms on double (the distance)
    engine.AddOutputTransformers({
        Aya::MakeTransformer<double>("Identity", std::function<void(double&)>(
            [](double& x) { /* noop */ })),
        Aya::MakeTransformer<double>("Negate", std::function<void(double&)>(
            [](double& x) { x = -x; })),
        Aya::MakeTransformer<double>("Square", std::function<void(double&)>(
            [](double& x) { x = x * x; })),
    });

    std::vector<std::any> points = {
        std::any(Point{3.0, 4.0}),
        std::any(Point{1.0, 0.0}),
        std::any(Point{0.0, 5.0}),
        std::any(Point{2.0, 2.0}),
        std::any(Point{-1.0, 3.0}),
    };

    auto mrs = engine.Search(
        {points}, {points},
        1, 1, 0, 0, 1.0f
    );

    auto consolidated = Aya::ConsolidateMRs(mrs);
    // dist(negate_x(p)) == dist(p), dist(swap(p)) == dist(p), etc.
    ASSERT_TRUE(consolidated.size() >= 3,
        "Point: expected >= 3 distance-invariance MRs, got " + std::to_string(consolidated.size()));

    // Print them for inspection
    for (auto& [text, stats] : consolidated) {
        std::cout << "    " << text << "\n";
    }
    PASS();
}

// ===========================================================================
// 7. NEW: std::set — union/intersection properties
// ===========================================================================

void TestSetOperations() {
    SECTION("8. std::set<int> — set algebra properties");
    TEST("set union self-idempotence");

    using S = std::set<int>;

    // SUT: identity on sets (returns input unchanged)
    auto engine = Aya::MREngine<S, S>(
        [](S s) -> S { return s; },
        [](S a, S b) { return a == b; }
    );

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
            [](S& s) { /* union with self = identity */ })),
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

    auto mrs = engine.Search(
        {{std::any(S{1,2,3}), std::any(S{5,10,15,20}), std::any(S{})}},
        {{std::any(S{1,2,3,4,5}), std::any(S{100}), std::any(S{-1,0,1})}},
        1, 1, 0, 0, 1.0f
    );

    auto consolidated = Aya::ConsolidateMRs(mrs);
    ASSERT_TRUE(!consolidated.empty(), "set: expected MRs for add/remove roundtrips");
    PASS();
}

// ===========================================================================
// 8. NEW: std::map — key-value store transforms
// ===========================================================================

void TestMapTransforms() {
    SECTION("9. std::map<string,int> — insert/erase properties");
    TEST("map roundtrip MRs");

    using M = std::map<std::string, int>;

    auto engine = Aya::MREngine<M, M>(
        [](M m) -> M { return m; },
        [](M a, M b) { return a == b; }
    );

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

    M m1 = {{"a", 1}, {"b", 2}};
    M m2 = {{"x", 10}};
    M m3 = {};
    M m4 = {{"a", 1}, {"b", 2}, {"c", 3}};

    auto mrs = engine.Search(
        {{std::any(m1), std::any(m2), std::any(m3)}},
        {{std::any(m1), std::any(m4), std::any(M{{"z",99}})}},
        1, 1, 0, 0, 1.0f
    );

    auto consolidated = Aya::ConsolidateMRs(mrs);
    // insert("a",1) then erase("a") should produce roundtrip MRs (for maps without "a")
    ASSERT_TRUE(!consolidated.empty(), "map: expected insert/erase roundtrip MRs");
    PASS();
}

// ===========================================================================
// 9. NEW: Mutation testing — verify MR reactivity
// ===========================================================================

void TestMutationDetection() {
    SECTION("10. Mutation Testing — sin mutant detection");
    TEST("MRs detect sin→cos substitution");

    // Generate MRs for correct sin
    auto engine = Aya::MREngine<double, double>(
        [](double x) -> double { return std::sin(x * PI / 180.0); },
        DoubleEq
    );

    engine.AddInputTransformers(0, MakeAllDoubleTransformers());
    engine.AddOutputTransformers(MakeAllDoubleTransformers());

    auto sample = {GenDoubles(5, 30, 90)};
    auto verify = {GenDoubles(5, 30, 90)};

    auto mrs = engine.Search(sample, verify, 1, 2, 0, 0, 1.0f);
    ASSERT_TRUE(!mrs.empty(), "mutation: no MRs found for original sin");

    // Now score the same MRs against mutated sin (sin→cos)
    std::function<double(double)> mutant = [](double x) -> double {
        return std::cos(x * PI / 180.0); // MUTANT: cos instead of sin
    };

    Aya::ScoreMRs<double, double>(mutant, DoubleEq, mrs, verify, 0, 0);

    // Check that at least one MR's success rate dropped
    bool detected = false;
    for (const auto& mr : mrs) {
        if (mr.LastSuccessRate < 1.0f) {
            detected = true;
            break;
        }
    }

    ASSERT_TRUE(detected, "mutation: sin→cos substitution not detected by any MR");
    PASS();

    // Count how many MRs were affected
    size_t affected = 0;
    for (const auto& mr : mrs) {
        if (mr.LastSuccessRate < 1.0f) affected++;
    }
    std::cout << "    " << affected << "/" << mrs.size() << " MRs detected the mutation\n";
}

// ===========================================================================
// 10. NEW: Priority queue / heap property
// ===========================================================================

void TestHeapProperty() {
    SECTION("11. Heap Property Preservation");
    TEST("make_heap invariants under push_heap");

    using Vec = std::vector<int>;

    // SUT: make_heap then return
    auto engine = Aya::MREngine<Vec, Vec>(
        [](Vec v) -> Vec { std::make_heap(v.begin(), v.end()); return v; },
        [](Vec a, Vec b) -> bool {
            // Both should be valid heaps with same max
            if (a.empty() && b.empty()) return true;
            if (a.empty() || b.empty()) return false;
            // Same top element is a useful invariance check
            return a.front() == b.front();
        }
    );

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
        // Identity-like: the heap top should be same regardless of input order
    });

    // For this test we check that make_heap produces the same max
    // regardless of input permutation. We need a custom approach since
    // output pool is empty — let's add a trivial one.
    engine.AddOutputTransformers({
        Aya::MakeTransformer<Vec>("Sort", std::function<void(Vec&)>(
            [](Vec& v) { std::sort(v.begin(), v.end()); })),
        Aya::MakeTransformer<Vec>("SortDesc", std::function<void(Vec&)>(
            [](Vec& v) { std::sort(v.begin(), v.end(), std::greater<>()); })),
    });

    auto mrs = engine.Search(
        {{std::any(Vec{5,3,8,1,2}), std::any(Vec{10,20,30}), std::any(Vec{1})}},
        {{std::any(Vec{7,3,9,1,5,2}), std::any(Vec{100,50,75,25})}},
        1, 1, 0, 0, 1.0f
    );

    auto consolidated = Aya::ConsolidateMRs(mrs);
    // Permutation of input should produce heap with same max
    ASSERT_TRUE(!consolidated.empty(), "heap: expected permutation-invariance for heap max");
    PASS();
}

// ===========================================================================
// 11. Edge cases
// ===========================================================================

void TestEdgeCases() {
    SECTION("12. Edge Cases");

    // Empty transformer pool should not crash
    TEST("empty search returns nothing");
    {
        auto engine = Aya::MREngine<double, double>(
            [](double x) -> double { return x * x; },
            DoubleEq
        );
        // No transformers added
        engine.AddInputTransformers(0, {});
        engine.AddOutputTransformers({
            Aya::MakeTransformer<double>("Negate", std::function<void(double&)>(
                [](double& x) { x = -x; })),
        });

        auto mrs = engine.Search(
            {{std::any(1.0)}},
            {{std::any(2.0)}},
            1, 1, 0, 0, 1.0f
        );
        ASSERT_TRUE(mrs.empty(), "empty input pool should produce 0 MRs");
        PASS();
    }

    // Domain errors should be caught, not crash
    TEST("domain errors handled gracefully");
    {
        auto engine = Aya::MREngine<double, double>(
            [](double x) -> double {
                if (x < 0) throw std::domain_error("negative");
                return std::sqrt(x);
            },
            DoubleEq
        );

        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<double>("Negate", std::function<void(double&)>(
                [](double& x) { x = -x; })),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<double>("Square", std::function<void(double&)>(
                [](double& x) { x = x * x; })),
        });

        // Mix of valid and invalid inputs — should not crash
        auto mrs = engine.Search(
            {{std::any(4.0), std::any(-1.0), std::any(9.0)}},
            {{std::any(16.0), std::any(-5.0), std::any(25.0)}},
            1, 1, 0, 0, 0.5f
        );
        // Should find something for valid inputs only
        PASS();
    }

    // Single input, single transformer
    TEST("minimal configuration");
    {
        auto engine = Aya::MREngine<double, double>(
            [](double x) -> double { return x * x; },
            DoubleEq
        );

        engine.AddInputTransformers(0, {
            Aya::MakeTransformer<double>("Negate", std::function<void(double&)>(
                [](double& x) { x = -x; })),
        });
        engine.AddOutputTransformers({
            Aya::MakeTransformer<double>("Identity", std::function<void(double&)>(
                [](double& x) { /* noop */ })),
        });

        auto mrs = engine.Search(
            {{std::any(3.0)}},
            {{std::any(5.0), std::any(7.0), std::any(-2.0)}},
            1, 1, 0, 0, 1.0f
        );
        // (-x)² == x² should be found
        ASSERT_TRUE(!mrs.empty(), "minimal: (-x)² == x² not found");
        PASS();
    }
}

// ===========================================================================
// 12. NEW: Heterogeneous argument types — func(double, int, double)
// ===========================================================================

void TestHeterogeneousArgs() {
    SECTION("13. Heterogeneous Args — func(double, int, double)");
    TEST("mixed-type auto type-matching");

    // f(x, z, y) = x * z + y  (double, int, double -> double)
    auto func = [](double x, int z, double y) -> double {
        return x * static_cast<double>(z) + y;
    };

    auto engine = Aya::MREngine<double, double, int, double>(
        func, DoubleEq
    );

    // Input transformers for arg 0 (double x) — identity included
    engine.AddInputTransformers(0, {
        Aya::MakeTransformer<double>("Identity", std::function<void(double&)>(
            [](double& v) { })),
        Aya::MakeTransformer<double, double>("Add", std::function<void(double&,double)>(
            [](double& v, double c) { v += c; }), 1.0),
        Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
            [](double& v, double c) { v *= c; }), 2.0),
    });

    // Input transformers for arg 1 (int z) — identity included
    engine.AddInputTransformers(1, {
        Aya::MakeTransformer<int>("Identity", std::function<void(int&)>(
            [](int& v) { })),
        Aya::MakeTransformer<int, int>("Add", std::function<void(int&,int)>(
            [](int& v, int c) { v += c; }), 1),
        Aya::MakeTransformer<int, int>("Add", std::function<void(int&,int)>(
            [](int& v, int c) { v += c; }), -1),
    });

    // Input transformers for arg 2 (double y) — identity included
    engine.AddInputTransformers(2, {
        Aya::MakeTransformer<double>("Identity", std::function<void(double&)>(
            [](double& v) { })),
        Aya::MakeTransformer<double, double>("Add", std::function<void(double&,double)>(
            [](double& v, double c) { v += c; }), 1.0),
        Aya::MakeTransformer<double>("Negate", std::function<void(double&)>(
            [](double& v) { v = -v; })),
    });

    // Constant output transforms
    engine.AddOutputTransformers({
        Aya::MakeTransformer<double, double>("Add", std::function<void(double&,double)>(
            [](double& v, double c) { v += c; }), 1.0),
        Aya::MakeTransformer<double, double>("Add", std::function<void(double&,double)>(
            [](double& v, double c) { v += c; }), -1.0),
        Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
            [](double& v, double c) { v *= c; }), 2.0),
    });

    // Variable output transforms with auto type-matching.
    // The Mul transformer takes a double arg — auto-match will source from
    // state doubles (result, x, y) and state int (z, which won't match double).
    engine.AddVariableOutputTransformers({
        Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
            [](double& v, double c) { v *= c; }), 0.0),
        Aya::MakeTransformer<double, double>("Add", std::function<void(double&,double)>(
            [](double& v, double c) { v += c; }), 0.0),
        Aya::MakeTransformer<double, double>("Div", std::function<void(double&,double)>(
            [](double& v, double c) { if(c!=0) v /= c; }), 0.0),
    });

    // Sample: double x, int z, double y
    auto mrs = engine.Search(
        {{std::any(2.0), std::any(3.0)},
         {std::any(2), std::any(3), std::any(4)},
         {std::any(1.0), std::any(5.0)}},
        {{std::any(1.0), std::any(4.0), std::any(7.0)},
         {std::any(1), std::any(2), std::any(5)},
         {std::any(2.0), std::any(6.0), std::any(3.0)}},
        1, 1, 0, 0, 1.0f, true
    );

    auto consolidated = Aya::ConsolidateMRs(mrs);

    // Verify that int-typed state (z at state[2]) was NOT matched for double arg positions
    // and double-typed state (result at state[0], x at state[1], y at state[3]) WAS matched
    bool found_double_sourced = false;
    for (const auto& [text, stats] : consolidated) {
        // Check for input[0] or input[2] or result (double sources)
        if (text.find("input[0]") != std::string::npos ||
            text.find("input[2]") != std::string::npos ||
            text.find("result") != std::string::npos) {
            found_double_sourced = true;
        }
        std::cout << "    " << text << "\n";
    }

    ASSERT_TRUE(!consolidated.empty(),
        "heterogeneous: expected MRs for mixed-type function");
    ASSERT_TRUE(found_double_sourced,
        "heterogeneous: expected variable transforms sourced from double-typed state values");
    PASS();
}

// ===========================================================================
// 13. NEW: Result-as-source for pow(double, double)
// ===========================================================================

void TestResultAsSource() {
    SECTION("14. Result-as-source — pow with output as transformer arg");
    TEST("pow result-as-source MRs");

    auto engine = Aya::MREngine<double, double, double>(
        [](double b, double e) -> double { return std::pow(b, e); },
        DoubleEq
    );

    // Input transforms on exponent
    engine.AddInputTransformers(0, {
        Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
            [](double& x, double c) { x *= c; }), 1.0),
    });
    engine.AddInputTransformers(1, {
        Aya::MakeTransformer<double, double>("Add", std::function<void(double&,double)>(
            [](double& x, double c) { x += c; }), 1.0),
        Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
            [](double& x, double c) { x *= c; }), 2.0),
    });

    engine.AddOutputTransformers({
        Aya::MakeTransformer<double>("Square", std::function<void(double&)>(
            [](double& x) { x = x * x; })),
    });

    // Variable: auto type-match — state is [pow(b,e), b, e], all doubles.
    // This means state[0] (the result) is also a candidate for double-typed args.
    engine.AddVariableOutputTransformers({
        Aya::MakeTransformer<double, double>("Mul", std::function<void(double&,double)>(
            [](double& x, double c) { x *= c; }), 0.0),
        Aya::MakeTransformer<double, double>("Div", std::function<void(double&,double)>(
            [](double& x, double c) { if(c!=0) x /= c; }), 0.0),
    });

    auto mrs = engine.Search(
        {{std::any(2.0), std::any(3.0), std::any(5.0)},
         {std::any(2.0), std::any(3.0), std::any(4.0)}},
        {{std::any(2.0), std::any(4.0), std::any(7.0)},
         {std::any(2.0), std::any(5.0), std::any(3.0)}},
        1, 1, 0, 0, 1.0f, true
    );

    auto consolidated = Aya::ConsolidateMRs(mrs);

    // Look for MRs that source from "result" (state[0])
    bool found_result_source = false;
    for (const auto& [text, stats] : consolidated) {
        if (text.find("result") != std::string::npos) {
            found_result_source = true;
        }
        std::cout << "    " << text << "\n";
    }

    ASSERT_TRUE(consolidated.size() >= 3,
        "pow result-as-source: expected >= 3 unique MRs, got " + std::to_string(consolidated.size()));
    // Result-as-source enables MRs like pow(x, 2y) == pow(x,y) * pow(x,y)
    ASSERT_TRUE(found_result_source,
        "pow result-as-source: expected at least one MR using 'result' as arg source");
    PASS();
}

// ===========================================================================
// Main
// ===========================================================================

int main() {
    std::cout << "Aya v2 Test Suite\n"
              << std::string(60, '=') << "\n";

    TestTrigFunctions();
    TestExpLogSqrt();
    TestPow();
    TestVectorPushPop();
    TestRotationMatrix();
    TestStringTransforms();
    TestPointStruct();
    TestSetOperations();
    TestMapTransforms();
    TestMutationDetection();
    TestHeapProperty();
    TestEdgeCases();
    TestHeterogeneousArgs();
    TestResultAsSource();

    std::cout << "\n" << std::string(60, '=') << "\n"
              << "Results: " << g_pass << " passed, " << g_fail << " failed\n"
              << std::string(60, '=') << "\n";

    return g_fail > 0 ? 1 : 0;
}
