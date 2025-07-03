#!/bin/bash

set -e

# Compiling for Linux
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cp build/bin/TimetableGenerator .

# Compiling for Windows
cmake -B build_win -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build_win -j
cp build_win/bin/TimetableGenerator.exe .

# Zipping the build
./reset_save_files.sh
./TimetableGenerator
echo $1 > version.txt
releases=()
for file in languages/*.txt; do
    lang=$(basename "$file" .txt)
    sed -i "s/^language=.*/language=$lang/" settings.txt
    releases+=("release_$lang.zip")
    zip release_$lang.zip TimetableGenerator TimetableGenerator.exe resources/* templates/ timetables/ languages/* logs/ LICENSE README.md settings.txt version.txt
done
rm TimetableGenerator TimetableGenerator.exe

# Creating a GitHub release
gh release create $1 ${releases[@]}
rm ${releases[@]}
