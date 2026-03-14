#!/usr/bin/env bash
# Run all benchmark configs from 1-2 up to 2-2.
# Usage: ./run_all_benchmarks.sh [output_file]
# If output_file is given, results are tee'd there.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "Configuring..."
cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR" -DAYA_BUILD_BENCHMARKS=ON

echo "Building benchmark..."
cmake --build "$BUILD_DIR" --target aya_par_bench

BENCH="$BUILD_DIR/aya_par_bench"

CONFIGS=(
    "1 2"
    "1 3"
    "2 1"
    "2 2"
)

run() {
    for cfg in "${CONFIGS[@]}"; do
        ic="${cfg%% *}"
        oc="${cfg##* }"
        echo ""
        echo "################################################################"
        echo "# CONFIG: ${ic}-${oc}"
        echo "################################################################"
        "$BENCH" "$ic" "$oc" --with-specific
    done
}

if [ "${1:-}" != "" ]; then
    run 2>&1 | tee "$1"
else
    run
fi

rm -rf build
