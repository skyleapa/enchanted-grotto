#!/bin/bash
if [ $# -eq 0 ] || [ $1 = "game" ]; then
    cmake -S . -B build -DBUILD_GAME=ON -DBUILD_TESTING=OFF
    cmake --build build
    cd build && ./enchanted_grotto
elif [ $1 = "test" ]; then
    cmake -S . -B build -DBUILD_GAME=OFF -DBUILD_TESTING=ON
    cmake --build build
    cd build && ctest --output-on-failure
else
    echo "Unrecognized argument, exiting..."
fi
#read -n 1 -s -r -p "Press any key to continue"