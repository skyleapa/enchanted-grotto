#!/bin/bash

echo "Building RmlUi..."
cd ext/RmlUi
cmake -B Build -S . -DBUILD_SHARED_LIBS=OFF
cmake --build Build -j
cd ..
cd ..

echo "Building Enchanted Grotto..."
if [ $1 = "test" ]; then
    cmake -S . -B build -DBUILD_GAME=OFF -DBUILD_TESTING=ON
    cmake --build build
    cd build && ctest --output-on-failure
else
    cmake -S . -B build -DBUILD_GAME=ON -DBUILD_TESTING=OFF
    cmake --build build
    cd build && ./enchanted_grotto
fi