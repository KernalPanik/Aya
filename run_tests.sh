#!/usr/bin/env bash
# Compiles and executes aya_tests.cpp

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST="$SCRIPT_DIR/aya_test"

echo "Compiling tests..."
clang++ -std=c++20 -O2 -pthread -o "$TEST" "$SCRIPT_DIR/aya_tests.cpp"

time $TEST