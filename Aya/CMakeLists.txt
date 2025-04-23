# Build the AyaTest Executable
cmake_minimum_required(VERSION 3.28)
project(Aya CXX)

set(CMAKE_CXX_STANDARD 26)

if(APPLE)
    set(LIBRARY_EXTENSION "dylib")
elseif(UNIX AND NOT APPLE)
    set(LIBRARY_EXTENSION "so")
elseif(WIN32)
    set(LIBRARY_EXTENSION "dll")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

include_directories(${CMAKE_SOURCE_DIR})
add_definitions(-w)

if (APPLE)
find_library(ACCELERATE_FRAMEWORK Accelerate)

    if (NOT ACCELERATE_FRAMEWORK)
        message(FATAL_ERROR "Accelerate not found")
    endif ()
endif ()

add_executable(AyaTest
        test/testMain.cpp
        test/framework/testRunner.cpp
        src/Common/util.cpp
        src/Common/util-tests.cpp
        src/Common/CartesianIterator.cpp
        src/Common/CompositeCartesianIterator.cpp
        src/Common/CartesianIteratorTests.cpp
        src/core/Modules/Transformer/transformer-tests.cpp)

add_executable(AyaSamples
        test/AyaSamples/main.cpp
        test/framework/testRunner.cpp
        src/Common/util.cpp
        src/Common/util-tests.cpp
        src/Common/CartesianIterator.cpp
        src/Common/CompositeCartesianIterator.cpp
        src/Common/CartesianIteratorTests.cpp
        src/core/Modules/Transformer/transformer-tests.cpp)

add_library(Aya SHARED
        src/Common/util.cpp
        src/Common/CartesianIterator.cpp
        src/Common/CompositeCartesianIterator.cpp)

if (APPLE)
target_link_libraries(AyaSamples ${ACCELERATE_FRAMEWORK})
endif ()