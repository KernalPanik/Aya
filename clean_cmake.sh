#!/bin/sh
# Remove CMake-generated files and directories in the root
rm -rf CMakeFiles CMakeCache.txt Makefile cmake_install.cmake *.dir

# Recursively remove CMake-generated files in src/samples subdirectories
find ./src/samples -type f \( -name 'CMakeLists.txt' -o -name 'CMakeCache.txt' -o -name 'Makefile' -o -name 'cmake_install.cmake' \) -exec rm -f {} +
find ./src/samples -type d \( -name 'CMakeFiles' -o -name '*.dir' \) -exec rm -rf {} +
