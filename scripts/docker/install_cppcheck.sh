#!/usr/bin/env bash

set -e

cd ~
git clone https://github.com/danmar/cppcheck.git
cd cppcheck
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j4
cmake --build . --target install
cd ..
rm -rf cppcheck
