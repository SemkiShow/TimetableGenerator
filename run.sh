#!/bin/bash

set -e

# Release build
if [ "$1" == "" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    ./build/bin/main
fi

# Debug build
if [ "$1" == "-d" ] || [ "$1" == "--debug" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug
    cmake --build build_debug -j
    gdb -ex run ./build_debug/bin/main
fi

# Windows build
if [ "$1" == "-w" ] || [ "$1" == "--windows" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build_win -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Release
    cmake --build build_win -j
    wine ./build_win/bin/main.exe
fi

# Help info
if [ "$1" == "--help" ]; then
    echo "Usage: ./run.sh [OPTION]..."
    echo "Compile and run the program"
    echo ""
    echo "With no OPTION, compile and run the release build"
    echo ""
    echo "-d, --debug    Compile the debug build and run it with gdb"
    echo "-w, --windows    Compile the Windows build and run it with Wine"
fi
