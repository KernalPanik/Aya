# Math Sample CMakeLists.txt
cmake_minimum_required(VERSION 3.28)

# Set the project name (will be prefixed with parent project name)
set(SAMPLE_NAME "math_sample")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR})

# Collect all source files
set(MATH_SAMPLE_SOURCES
    main.cpp
    DoubleTypeTransformers.hpp
    InputGenerator.hpp
    LinalgTest.hpp
    LinalgTransformers.hpp
    StandardMathTest.hpp
    Stopwatch.hpp
    MathMutants.hpp
)

# Create executable
add_executable(${SAMPLE_NAME} ${MATH_SAMPLE_SOURCES})

# Set C++ standard
target_compile_features(${SAMPLE_NAME} PRIVATE cxx_std_20)

# Compiler flags for optimization and debugging
target_compile_options(${SAMPLE_NAME} PRIVATE
    -O3
    -g
    -ffast-math
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# Platform-specific libraries
if(APPLE)
    find_library(ACCELERATE_FRAMEWORK Accelerate)
    if(ACCELERATE_FRAMEWORK)
        target_link_libraries(${SAMPLE_NAME} PRIVATE ${ACCELERATE_FRAMEWORK})
    else()
        message(WARNING "Accelerate framework not found on macOS")
    endif()
    
    # Additional macOS libraries
    target_link_libraries(${SAMPLE_NAME} PRIVATE z iconv)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${SAMPLE_NAME} PRIVATE z)
elseif(WIN32)
    # Windows-specific libraries if needed
endif()

# Set output directory
set_target_properties(${SAMPLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples/math
)