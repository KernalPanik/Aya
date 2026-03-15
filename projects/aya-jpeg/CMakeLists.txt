cmake_minimum_required(VERSION 3.20)
project(aya-jpeg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Aya header location (parent repo)
set(AYA_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../src" CACHE PATH "Path to Aya headers")

# Find libjpeg-turbo
# Try pkg-config first, fall back to Homebrew / manual paths
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(TURBOJPEG QUIET libturbojpeg)
endif()

if(NOT TURBOJPEG_FOUND)
    # Try Homebrew path on macOS
    if(APPLE)
        set(TURBOJPEG_HINT "/opt/homebrew/opt/jpeg-turbo")
        if(NOT EXISTS "${TURBOJPEG_HINT}/include/turbojpeg.h")
            set(TURBOJPEG_HINT "/usr/local/opt/jpeg-turbo")
        endif()
    else()
        set(TURBOJPEG_HINT "/usr")
    endif()

    find_path(TURBOJPEG_INCLUDE_DIRS turbojpeg.h
        HINTS ${TURBOJPEG_HINT}/include /usr/include)
    find_library(TURBOJPEG_LIBRARIES turbojpeg
        HINTS ${TURBOJPEG_HINT}/lib /usr/lib /usr/lib/x86_64-linux-gnu)

    if(TURBOJPEG_INCLUDE_DIRS AND TURBOJPEG_LIBRARIES)
        set(TURBOJPEG_FOUND TRUE)
    else()
        message(FATAL_ERROR
            "libjpeg-turbo not found. Install it:\n"
            "  macOS:  brew install jpeg-turbo\n"
            "  Ubuntu: sudo apt-get install libturbojpeg0-dev")
    endif()
endif()

add_executable(jpeg_mr_test jpeg_mr_test.cpp)

target_include_directories(jpeg_mr_test PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party
    ${AYA_INCLUDE_DIR}
    ${TURBOJPEG_INCLUDE_DIRS}
)

target_link_libraries(jpeg_mr_test PRIVATE
    ${TURBOJPEG_LIBRARIES}
)

if(TURBOJPEG_LIBRARY_DIRS)
    target_link_directories(jpeg_mr_test PRIVATE
        ${TURBOJPEG_LIBRARY_DIRS}
    )
endif()

target_compile_options(jpeg_mr_test PRIVATE -O2)
