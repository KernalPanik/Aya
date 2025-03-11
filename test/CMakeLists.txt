cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 17)
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
add_executable(AyaTest
        testMain.cpp
        framework/testRunnerTests.cpp
        framework/testRunner.cpp
        ../src/Common/util.cpp
        ../src/Common/util-tests.cpp
        ../src/Common/CartesianIterator.cpp
        ../src/Common/CompositeCartesianIterator.cpp
        ../src/Common/CartesianIteratorTests.cpp
        ../src/core/Modules/MRGenerator/TestContextTests.cpp
        ../src/core/Modules/MRGenerator/MRBuilder.cpp
        ../src/core/metamorphicRelation-tests.cpp
        ../src/core/Modules/Transformer/transformer-tests.cpp)
target_link_libraries(AyaTest m)