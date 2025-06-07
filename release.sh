#!/bin/bash

set -e

# Compiling for Linux
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cp build/bin/main .

# Compiling for Windows
cmake -B build_win -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build_win -j
cp build_win/bin/main.exe .

# Zipping the build
./reset_save_files.sh
zip release.zip main main.exe resources/* templates/ timetables/ LICENSE README.md settings.txt
rm main main.exe

# Creating a GitHub release
gh release create $1 release.zip
rm release.zip
