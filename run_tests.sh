#!/bin/bash
cmake -S . -B build -DBUILD_GAME=OFF -DBUILD_TESTING=ON
cmake --build build
cd build && ctest --output-on-failure