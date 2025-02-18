#!/bin/bash
cmake -S . -B build -DBUILD_GAME=ON -DBUILD_TESTING=OFF
cmake --build build
cd build && ./enchanted_grotto