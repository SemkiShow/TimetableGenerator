#!/bin/bash

set -e

# Release build
if [ "$1" == "" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j32
    ./build/bin/TimetableGenerator
fi

# Debug build
if [ "$1" == "-d" ] || [ "$1" == "--debug" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug
    cmake --build build_debug -j32
    gdb -ex run ./build_debug/bin/TimetableGenerator
fi

# Windows build
if [ "$1" == "-w" ] || [ "$1" == "--windows" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build_win -DCMAKE_TOOLCHAIN_FILE="$(pwd)/mingw-w64-x86_64.cmake" -DCMAKE_BUILD_TYPE=Release
    cmake --build build_win -j32
    wine ./build_win/bin/TimetableGenerator.exe
fi

# Profile build
if [ "$1" == "-p" ] || [ "$1" == "--profile" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build_profile -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -pg"
    cmake --build build_profile -j
    ./build_profile/bin/TimetableGenerator
    gprof ./build_profile/bin/TimetableGenerator > profile.txt
fi

# Memory leak build
if [ "$1" == "-m" ] || [ "$1" == "--memory-leak" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build_memory_leak -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -fsanitize=address"
    cmake --build build_memory_leak -j
    ./build_memory_leak/bin/TimetableGenerator
fi

# Help info
if [ "$1" == "--help" ]; then
    echo "Usage: ./run.sh [OPTION]..."
    echo "Compile and run the program"
    echo ""
    echo "With no OPTION, compile and run the release build"
    echo ""
    echo "-d, --debug      Compile the debug build and run it with gdb"
    echo "-w, --windows    Compile the Windows build and run it with Wine"
    echo "-p, --profile    Compile the profile build and run it with gprof"
    echo "-m, --memory-leak    Compile the memory leak build and run it"
fi
