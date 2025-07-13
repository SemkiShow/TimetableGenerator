#!/bin/bash

set -e
shopt -s globstar

if [ -z "$1" ]; then
    echo "You must specify the release version!"
    exit 1
fi

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
systems=("linux" "windows")
for file in resources/languages/*.txt; do
    lang=$(basename "$file" .txt)
    sed -i "s/^language=.*/language=$lang/" settings.txt
    for system in "${systems[@]}"; do
        releases+=("release-$system-$lang.zip")
        zip release-$system-$lang.zip resources/** templates/ timetables/ logs/ LICENSE README.md settings.txt version.txt
        if [ $system == "linux" ]; then
            zip release-$system-$lang.zip TimetableGenerator
        fi
        if [ $system == "windows" ]; then
            zip release-$system-$lang.zip TimetableGenerator.exe
        fi
    done
done
rm TimetableGenerator TimetableGenerator.exe

# Creating a GitHub release
gh release create $1 ${releases[@]}
rm ${releases[@]}
