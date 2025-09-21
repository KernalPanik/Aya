# Build the Aya Library and Samples
cmake_minimum_required(VERSION 3.28)
project(Aya CXX)

set(CMAKE_CXX_STANDARD 26)

# Automatically discover and build all sample projects
file(GLOB SAMPLE_DIRS "${CMAKE_SOURCE_DIR}/src/samples/*")
foreach(SAMPLE_DIR ${SAMPLE_DIRS})
    if(IS_DIRECTORY ${SAMPLE_DIR})
        get_filename_component(SAMPLE_NAME ${SAMPLE_DIR} NAME)
        if(EXISTS "${SAMPLE_DIR}/CMakeLists.txt")
            message(STATUS "Adding sample: ${SAMPLE_NAME}")
            add_subdirectory(${SAMPLE_DIR})
        endif()
    endif()
endforeach()