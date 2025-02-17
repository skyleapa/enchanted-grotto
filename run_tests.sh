#!/bin/bash
rm -rf build
mkdir build
cd build
cmake .. -DBUILD_GAME=OFF -DBUILD_TESTING=ON
make
ctest --output-on-failure