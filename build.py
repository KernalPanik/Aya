import os
import sys
import shutil
import subprocess


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


def prep_cmake(should_we_test: bool, should_we_release: bool):
    if (should_we_release and should_we_test):
        print("Both Release and Test flags enabled somehow. This should not happen")
        exit(1)
    if should_we_test:
        print("Building Tests... (Debug Mode ON)")
        subprocess.run(["cmake", "-DCMAKE_BUILD_TYPE=Debug", "-DTEST=true", "CMakeLists.txt"])
    elif should_we_release:
        print("Building Executable...")
        subprocess.run(["cmake", "-DTEST=false", "CMakeLists.txt"])
    else:
        print("Building Standard Debug Executable...")
        subprocess.run(["cmake", "-DCMAKE_BUILD_TYPE=Debug", "-DTEST=false", "CMakeLists.txt"])


def run_make():
    subprocess.run(["make"])


def build_entry(should_we_test: bool, should_we_release: bool):
    if "posix" in os.name:
        cleanup_cmake_artifacts()
        prep_cmake(should_we_test, should_we_release)
        run_make()
        print("Done")
    elif "win32" in os.name:
        pass  # TODO


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

    should_we_test = False
    if arg.lower() == "test":
        should_we_test = True

    build_entry(should_we_test, should_we_release)
