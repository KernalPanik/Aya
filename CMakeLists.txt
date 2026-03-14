cmake_minimum_required(VERSION 3.20)
project(Aya VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

# Header-only interface library — the main consumable target
add_library(aya INTERFACE)
target_include_directories(aya INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(aya INTERFACE cxx_std_20)

# Parallel extension (also header-only, requires pthreads)
add_library(aya_parallel INTERFACE)
target_link_libraries(aya_parallel INTERFACE aya Threads::Threads)

# Optional targets (demo, tests, benchmarks)
option(AYA_BUILD_TESTS "Build the test suite" OFF)
option(AYA_BUILD_BENCHMARKS "Build the parallel benchmark" OFF)

if(AYA_BUILD_TESTS)
    add_executable(aya_tests src/aya_tests.cpp)
    target_link_libraries(aya_tests PRIVATE aya Threads::Threads)
    target_compile_options(aya_tests PRIVATE -O2)

    enable_testing()
    add_test(NAME aya_tests COMMAND aya_tests)
endif()

if(AYA_BUILD_BENCHMARKS)
    add_executable(aya_par_bench projects/benchmarks/aya_parallel_benchmark.cpp)
    target_link_libraries(aya_par_bench PRIVATE aya_parallel)
    target_compile_options(aya_par_bench PRIVATE -O2)
endif()

# Install rules
install(FILES src/aya.hpp src/aya_parallel.hpp
    DESTINATION include/aya
)
install(TARGETS aya aya_parallel
    EXPORT AyaTargets
    INCLUDES DESTINATION include
)
install(EXPORT AyaTargets
    FILE AyaConfig.cmake
    NAMESPACE Aya::
    DESTINATION lib/cmake/aya
)
