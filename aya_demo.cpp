#include "aya_parallel.hpp"

#include <cmath>

int main() {
    using namespace Aya;

    // --- sin(x) demo ---
    std::cout << std::string(60, '=') << "\n"
              << "MR Search: sin(x)\n"
              << std::string(60, '=') << "\n\n";

    auto sin_engine = ParallelMREngine<double, double>(
        [](double x) -> double { return std::sin(x); },
        [](double a, double b) { return std::abs(a - b) <= 1e-9; }
    );

    // Input transformers for arg 0
    sin_engine.AddInputTransformers(0, {
        MakeTransformer<double>("Negate",  std::function<void(double&)>([](double& x) { x = -x; })),
        MakeTransformer<double>("AddPi",   std::function<void(double&)>([](double& x) { x += M_PI; })),
        MakeTransformer<double>("Add2Pi",  std::function<void(double&)>([](double& x) { x += 2.0 * M_PI; })),
        MakeTransformer<double>("SubPi",   std::function<void(double&)>([](double& x) { x -= M_PI; })),
    });

    // Output transformers
    sin_engine.AddOutputTransformers({
        MakeTransformer<double>("Negate", std::function<void(double&)>([](double& x) { x = -x; })),
        MakeTransformer<double>("Abs",    std::function<void(double&)>([](double& x) { x = std::abs(x); })),
        MakeTransformer<double>("Square", std::function<void(double&)>([](double& x) { x = x * x; })),
    });

    auto sin_mrs = sin_engine.Search(
        /*sample*/  {{std::any(0.3), std::any(0.7), std::any(1.1), std::any(2.5)}},
        /*verify*/  {{std::any(0.2), std::any(0.9), std::any(1.5), std::any(3.0), std::any(0.05)}},
        /*input_chain*/  1,
        /*output_chain*/ 1,
        /*left*/  0, /*right*/ 0,
        /*min_success*/ 1.0f
    );

    std::cout << "Found " << sin_mrs.size() << " MRs with 100% success:\n\n";
    DumpMRsToStdout(sin_mrs, 1.0f);

    // --- pow(base, exp) demo with variable output transforms ---
    std::cout << "\n" << std::string(60, '=') << "\n"
              << "MR Search: pow(base, exp) — with implicit output transforms\n"
              << std::string(60, '=') << "\n\n";

    auto pow_engine = ParallelMREngine<double, double, double>(
        [](double base, double exp) -> double { return std::pow(base, exp); },
        [](double a, double b) { return std::abs(a - b) <= 1e-6; }
    );

    pow_engine.AddInputTransformers(0, {
        MakeTransformer<double>("Mul", std::function<void(double&, double)>(
            [](double& x, double c) { x *= c; }), 1.0),
    });
    pow_engine.AddInputTransformers(1, {
        MakeTransformer<double>("Add", std::function<void(double&, double)>(
            [](double& x, double c) { x += c; }), 1.0),
        MakeTransformer<double>("Add", std::function<void(double&, double)>(
            [](double& x, double c) { x += c; }), -1.0),
        MakeTransformer<double>("Add", std::function<void(double&, double)>(
            [](double& x, double c) { x += c; }), 2.0),
        MakeTransformer<double>("Mul", std::function<void(double&, double)>(
            [](double& x, double c) { x *= c; }), 2.0),
    });

    // Constant output transforms
    pow_engine.AddOutputTransformers({
        MakeTransformer<double>("Square", std::function<void(double&)>(
            [](double& x) { x = x * x; })),
    });

    // Variable output transforms: Mul and Div, args overridden from state
    pow_engine.AddVariableOutputTransformers(
        {
            MakeTransformer<double>("Mul", std::function<void(double&, double)>(
                [](double& x, double c) { x *= c; }), 0.0),
            MakeTransformer<double>("Div", std::function<void(double&, double)>(
                [](double& x, double c) { if (c != 0) x /= c; }), 0.0),
        },
        {
            {0},  // Mul: override arg from state index 0 (= arg 0 = base)
            {0},  // Div: override arg from state index 0 (= arg 0 = base)
        }
    );

    auto pow_mrs = pow_engine.Search(
        /*sample*/  {{std::any(2.0), std::any(3.0), std::any(5.0)},
                     {std::any(2.0), std::any(3.0), std::any(4.0)}},
        /*verify*/  {{std::any(2.0), std::any(4.0), std::any(7.0)},
                     {std::any(2.0), std::any(5.0), std::any(3.0)}},
        /*input_chain*/  1,
        /*output_chain*/ 1,
        /*left*/  0, /*right*/ 0,
        /*min_success*/ 1.0f,
        /*override_args*/ true
    );

    std::cout << "Found " << pow_mrs.size() << " MRs with 100% success:\n\n";
    DumpMRsToStdout(pow_mrs, 1.0f);

    return 0;
}
