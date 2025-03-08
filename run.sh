#!/bin/bash

# Ensure VCPKG_ROOT is set
if [ -z "$VCPKG_ROOT" ]; then
    echo "Error: VCPKG_ROOT is not set. Please install vcpkg and set it before running this script."
    exit 1
fi

if [ $# -eq 0 ] || [ $1 = "game" ]; then
    cmake -S . -B build -DBUILD_GAME=ON -DBUILD_TESTING=OFF -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
    cmake --build build
    cd build && ./enchanted_grotto
elif [ $1 = "test" ]; then
    cmake -S . -B build -DBUILD_GAME=OFF -DBUILD_TESTING=ON -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
    cmake --build build
    cd build && ctest --output-on-failure
else
    echo "Unrecognized argument, exiting..."
fi