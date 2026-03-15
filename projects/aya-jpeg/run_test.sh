#!/usr/bin/env bash
set -euo pipefail

cmake -DCMAKE_BUILD_TYPE=Debug build
cmake --build build

PSNR_VALUES=(20 25 30 35 40)

for psnr in "${PSNR_VALUES[@]}"; do
    echo "=== Running with PSNR threshold: ${psnr} dB ==="
    ./build/jpeg_mr_test --psnr "$psnr" "$@"
done
