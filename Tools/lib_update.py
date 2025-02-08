# libAyaCore updater script used to publish final library to a specified location
# Library location is controlled through an argument or an Environment Variable <AYA_LIB_PATH>

import os
import shutil
from pathlib import Path

from sys import platform

def src_path() -> str:
    return os.path.join(repo_root(), "src")

def cwd_does_look_like_repo_root() -> bool:
    dir_list = os.listdir(os.getcwd())
    if "build.py" in dir_list:
        return True
    
    return False

def repo_root() -> str:
    if cwd_does_look_like_repo_root():
        return os.getcwd()
    return os.path.dirname(os.getcwd())

if __name__ == "__main__":
    target_path = os.path.join(repo_root(), "artifacts")
    if os.environ.get('AYA_LIB_PATH') is None:
        print("AYA_LIB_PATH is not set, will store artifacts at {}".format(repo_root()))
    else:
        target_path = os.environ['AYA_LIB_PATH']

    print("Moving produced library to a specified path {}".format(target_path))

    library_path = ""

    if platform == "darwin":
            library_path = os.path.join(src_path(), "libAyaCore.dylib")
    elif platform == "linux" or platform == "linux2":
            library_path = os.path.join(src_path(), "libAyaCore.so")
    elif platform == "win32":
           library_path = os.path.join(src_path(), "libAyaCore.dll")

    header_path = os.path.join(src_path(), "public", "AyaCore.h")


    new_lib_path = ""

    if platform == "darwin":
        new_lib_path = os.path.join(target_path, "libAyaCore.dylib")
    elif platform == "linux" or platform == "linux2":
        new_lib_path = os.path.join(target_path, "libAyaCore.so")
    elif platform == "win32":
        new_lib_path = os.path.join(target_path, "libAyaCore.dll")

    new_header_path = os.path.join(target_path, "AyaCore.h")

    Path(target_path).mkdir(parents=True, exist_ok=True)

    print("Moving library to {}".format(new_lib_path))
    shutil.move(library_path, new_lib_path)
    print("Copying header to {}".format(new_header_path))
    shutil.copy(header_path, new_header_path)