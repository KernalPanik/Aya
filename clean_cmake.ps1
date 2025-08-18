# Remove CMake-generated files and directories in the root
Remove-Item -Recurse -Force CMakeFiles, CMakeCache.txt, Makefile, cmake_install.cmake, *.dir -ErrorAction SilentlyContinue

# Recursively remove CMake-generated files in src/samples subdirectories
Get-ChildItem -Path ./src/samples -Recurse -Include CMakeLists.txt,CMakeCache.txt,Makefile,cmake_install.cmake | Remove-Item -Force -ErrorAction SilentlyContinue
Get-ChildItem -Path ./src/samples -Recurse -Directory -Include CMakeFiles,*.dir | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
