#!/usr/bin/env bash

set -e

cd ~
git clone https://github.com/include-what-you-use/include-what-you-use.git
cd include-what-you-use
git checkout clang_9.0
cd ..
mkdir build && cd $_
cmake -G "Unix Makefiles" -DIWYU_LLVM_ROOT_PATH=/usr/lib/llvm-9.0 ../include-what-you-use
make -j4
make install
