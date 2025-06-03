#!/bin/bash

set -e

# Release build
if [ "$1" == "" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build
    cmake --build build -j ${nproc}
    ./build/bin/main
fi

# Debug build
if [ "$1" == "-d" ] || [ "$1" == "--debug" ]; then
    clear
    ./reset_save_files.sh --soft
    cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug
    cmake --build build_debug -j ${nproc}
    gdb -ex run ./build_debug/bin/main
fi

# Web build
if [ "$1" == "-w" ] || [ "$1" == "--web" ]; then
    clear
    ./reset_save_files.sh --soft
    if [ "$2" == "-m" ] || [ "$2" == "--minimal" ]; then
        emcmake cmake -B build_web -DPLATFORM=Web -DSHELL=Minimal
    else
        emcmake cmake -B build_web -DPLATFORM=Web -DSHELL=Full
    fi
    cmake --build build_web -j ${nproc}
    emrun ./build_web/bin/main.html
fi

# Help info
if [ "$1" == "--help" ]; then
    echo "Usage: ./run.sh [OPTION]..."
    echo "Compile and run the program"
    echo ""
    echo "With no OPTION, compile and run the release build"
    echo ""
    echo "-d, --debug    Compile the debug build and run it with gdb"
    echo "-w, --web      Compile the WebAssembly build"
    echo "-m, --minimal  Compile a WebAssembly build with the minimal shell"
fi
