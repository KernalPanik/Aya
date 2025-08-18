# Build the Aya Library and Samples
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

# Global include directories and definitions
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
add_definitions(-w)
add_definitions(-lz)
add_definitions(-liconv)
add_definitions(-ffast-math)
add_definitions(-o3)
add_definitions(-g)

# Find required libraries
find_library(ICONV iconv)
if (NOT ICONV)
    message(FATAL_ERROR "Failed to locate libiconv")
endif()

if (APPLE)
    find_library(ACCELERATE_FRAMEWORK Accelerate)
    if (NOT ACCELERATE_FRAMEWORK)
        message(FATAL_ERROR "Accelerate not found")
    endif ()
endif ()

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