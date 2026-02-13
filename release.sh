#!/bin/bash

set -e
shopt -s globstar
executable_name=TimetableGenerator

if [ -z "$1" ]; then
    echo "You must specify the release version!"
    exit 1
fi

# Compiling for Linux
mkdir -p build_release
cmake -B build_release -DCMAKE_BUILD_TYPE=Release
cmake --build build_release -j$(nproc)
cp build_release/bin/$executable_name .

# Compiling for Windows
mkdir -p build_release_windows
cmake -B build_release_windows -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build_release_windows -j$(nproc)
cp build_release_windows/bin/$executable_name.exe .

# Zipping the build
rm settings.txt
./$executable_name
echo $1 > version.txt
systems=("linux" "windows")
archive_name=$executable_name-$1
zip $archive_name.zip resources/** LICENSE README.md version.txt
for lang in resources/locales/*; do
    sed -i "s/^language=.*/language=$lang/" settings.txt
    for system in "${systems[@]}"; do
        cp $archive_name.zip $archive_name-$system-$lang.zip
        zip $archive_name-$system-$lang.zip settings.txt
        if [ $system == "linux" ]; then
            zip $archive_name-$system-$lang.zip $executable_name
        elif [ $system == "windows" ]; then
            zip $archive_name-$system-$lang.zip $executable_name.exe
        fi
    done
done
rm $executable_name $executable_name.exe $archive_name.zip

# Creating a GitHub release
gh release create $1 $archive_name*
rm $archive_name* settings.txt
