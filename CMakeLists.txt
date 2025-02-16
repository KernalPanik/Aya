cmake_minimum_required(VERSION 3.28)
project(AyaCore CXX)

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

add_definitions(-w)

add_subdirectory(src)
add_subdirectory(test)