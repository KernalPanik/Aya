#!/usr/bin/env bash
# Builds and runs aya_tests via CMake

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "Configuring..."
cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR" -DAYA_BUILD_TESTS=ON

echo "Building tests..."
cmake --build "$BUILD_DIR" --target aya_tests

echo ""
./build/aya_tests

rm -rf build