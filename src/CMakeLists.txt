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

include_directories(${PROJECT_SOURCE_DIR})
add_library(AyaCore SHARED Common/util.cpp Common/CartesianIterator.cpp)
target_link_libraries(AyaCore m)