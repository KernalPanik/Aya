cmake_minimum_required(VERSION 3.30)
project(AyaSamples)

set(CMAKE_CXX_STANDARD 20)

find_library(LIBR
    NAMES Aya
    PATHS
    ${CMAKE_SOURCE_DIR}/lib)

add_executable(AyaSamples main.cpp)

target_link_libraries(AyaSamples PRIVATE ${LIBR})