import os
import sys
import shutil
import subprocess
import platform

def cleanup_cmake_artifacts():
    files_to_remove = ["CMakeCache.txt", "aya", "ayatest", "cmake_install.cmake", "makefile"]
    directories_to_remove = ["CMakeFiles"]

    print("Removing CMake artifacts (if present)")
    for f in files_to_remove:
        if os.path.isfile(f):
            os.remove(f)
    for d in directories_to_remove:
        if os.path.isdir(d):
            shutil.rmtree(d)


def prep_cmake(should_we_release: bool):
    if should_we_release:
        print("Building Executable...")
        subprocess.run(["cmake", "-DTEST=false", "CMakeLists.txt"])
    else:
        print("Building Standard Debug Executable...")
        subprocess.run(["cmake", "-DCMAKE_BUILD_TYPE=Debug", "-DTEST=false", "CMakeLists.txt"])


def run_make():
    subprocess.run(["make"])


def build_entry(should_we_release: bool):
    if "posix" in os.name:
        cleanup_cmake_artifacts()
        prep_cmake(should_we_release)
        run_make()
        print("Done")
    elif "win32" in os.name:
        pass  # TODO

def getListOfSourceFiles() -> list:
    return [
        "src/public/transformer.hpp",
        "src/public/TransformBuilder.hpp",
        "src/public/MRBuilder.hpp",
        "src/public/MetamorphicRelation.hpp",
        "src/public/MRContext.hpp",
        "src/public/CoreUtilities.hpp",
        "src/public/CartesianIterator.h",
        "src/public/CompositeCartesianIterator.h",
        "src/public/AyaCore.h",
    ]

def package_products():
    print("Packaging Final Products!")
    cwd = os.getcwd()
    libPath = os.path.join(cwd, "libAya")
    targetLocation = os.path.join(cwd, "ayaBuild")

    src_files = getListOfSourceFiles()

    if platform.system().lower() == "darwin":
        libPath = libPath + ".dylib"
    elif platform.system().lower() == "linux":
        libPath = libPath + ".so"
    else:
        # yuck
        libPath = libPath + ".dll"

    if not os.path.exists(targetLocation):
        os.makedirs(targetLocation)
    shutil.copy(libPath, targetLocation)
    for f in src_files:
        shutil.copy(f, targetLocation)

    print("You will find the final build in ayaBuild directory.")

if __name__ == "__main__":
    arg = ""
    if len(sys.argv) > 1:
        arg = sys.argv[1]

    if arg.lower() == "cache":
        print("deleting the cmake cache and other build artifacts")
        cleanup_cmake_artifacts()
        exit(0)

    should_we_release = False
    if arg.lower() == "release":
        print("Preparing Release Build")
        cleanup_cmake_artifacts()
        should_we_release = True;

    build_entry(should_we_release)
    package_products()