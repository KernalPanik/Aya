cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 14)

if(APPLE)
    set(LIBRARY_EXTENSION "dylib")
elseif(UNIX AND NOT APPLE)
    set(LIBRARY_EXTENSION "so")
elseif(WIN32)
    set(LIBRARY_EXTENSION "dll")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

add_library(AyaCore SHARED common/util.cpp)
target_link_libraries(AyaCore m)