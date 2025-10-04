#!/bin/bash

set -e
shopt -s globstar

if [ -z "$1" ]; then
    echo "You must specify the release version!"
    exit 1
fi

# Compiling for Linux
mkdir -p build_release
cmake -B build_release -DCMAKE_BUILD_TYPE=Release
cmake --build build_release -j${nproc}
cp build_release/bin/TimetableGenerator .

# Compiling for Windows
mkdir -p build_release_windows
cmake -B build_release_windows -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build_release_windows -j${nproc}
cp build_release_windows/bin/TimetableGenerator.exe .

# Zipping the build
rm settings.txt
mkdir -p timetables templates
./TimetableGenerator
echo $1 > version.txt
releases=()
systems=("linux" "windows")
for file in resources/languages/*.txt; do
    lang=$(basename "$file" .txt)
    sed -i "s/^language=.*/language=$lang/" settings.txt
    for system in "${systems[@]}"; do
        releases+=("release-$system-$lang.zip")
        zip release-$system-$lang.zip resources/** templates/ timetables/ logs/ LICENSE README.md version.txt settings.txt
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
rm ${releases[@]} settings.txt
