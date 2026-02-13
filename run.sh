#!/bin/bash

set -e
executable_name=TimetableGenerator

# Release build
if [ "$1" == "" ]; then
    clear
    cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build build -j$(nproc)
    ./build/bin/$executable_name
fi

# Debug build
if [ "$1" == "-d" ] || [ "$1" == "--debug" ]; then
    clear
    cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug
    cmake --build build_debug -j$(nproc)
    gdb -ex run --args ./build_debug/bin/$executable_name
fi

# Windows build
if [ "$1" == "-w" ] || [ "$1" == "--windows" ]; then
    clear
    cmake -B build_windows -DCMAKE_TOOLCHAIN_FILE="$(pwd)/mingw-w64-x86_64.cmake" -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build build_windows -j$(nproc)
    wine ./build_windows/bin/$executable_name.exe
fi

# Profile build
if [ "$1" == "-p" ] || [ "$1" == "--profile" ]; then
    clear
    cmake -B build_profile -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-pg"
    cmake --build build_profile -j$(nproc)
    perf record -g --call-graph dwarf ./build_profile/bin/$executable_name
    hotspot perf.data
fi

# Memory leak build
if [ "$1" == "-m" ] || [ "$1" == "--memory-leak" ]; then
    clear
    cmake -B build_memory_leak -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS="-fsanitize=address"
    cmake --build build_memory_leak -j$(nproc)
    ./build_memory_leak/bin/$executable_name
fi

# Help info
if [ "$1" == "--help" ]; then
    echo "Usage: ./run.sh [OPTION]..."
    echo "Compile and run $executable_name"
    echo ""
    echo "With no OPTION, compile and run the release build"
    echo ""
    echo "-d, --debug      Compile the debug build and run it with gdb"
    echo "-w, --windows    Compile the Windows build and run it with Wine"
    echo "-p, --profile    Compile the profile build, profile it with perf and display the data with hotspot"
    echo "-m, --memory-leak    Compile the memory leak build and run it"
fi
