# STL Sample CMakeLists.txt
cmake_minimum_required(VERSION 3.28)

# Set the project name (will be prefixed with parent project name)
set(SAMPLE_NAME "stl_sample")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR})

# Collect all source files
set(STL_SAMPLE_SOURCES
    main.cpp
    StlTests.hpp
    TransformerFunctions.hpp
)

# Create executable
add_executable(${SAMPLE_NAME} ${STL_SAMPLE_SOURCES})

# Set C++ standard
target_compile_features(${SAMPLE_NAME} PRIVATE cxx_std_20)

# Compiler flags for optimization and debugging
target_compile_options(${SAMPLE_NAME} PRIVATE
    -O3
    -g
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# Platform-specific libraries (STL sample likely needs fewer dependencies)
if(UNIX AND NOT APPLE)
    # Link standard libraries if needed
    target_link_libraries(${SAMPLE_NAME} PRIVATE pthread)
elseif(WIN32)
    # Windows-specific libraries if needed
endif()

# Set output directory
set_target_properties(${SAMPLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/samples/stl
)
