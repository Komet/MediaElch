#!/usr/bin/env bash

set -e

cd ~
git clone https://github.com/include-what-you-use/include-what-you-use.git
cd include-what-you-use
git checkout clang_10
cd ..
mkdir build && cd $_
cmake -G "Unix Makefiles" -DIWYU_LLVM_ROOT_PATH=/usr/lib/llvm-10 ../include-what-you-use
make -j4
make install
