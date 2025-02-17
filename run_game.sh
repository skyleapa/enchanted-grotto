#!/bin/bash
rm -rf build
mkdir build
cd build
cmake .. -DBUILD_GAME=ON -DBUILD_TESTING=OFF
make
./enchanted_grotto