#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/.." > /dev/null 2>&1
source scripts/utils.sh

print_important "Building with clang-tidy and applying fixes"

rm -rf build/clang-tidy
mkdir -p build/clang-tidy
cd build/clang-tidy

print_info "clang-tidy build dir: $(pwd)"

export CXX=clang++
export CC=clang

cmake -DCMAKE_BUILD_TYPE=Debug \
	-DENABLE_CLANG_TIDY_FIX=ON \
	-DENABLE_COLOR_OUTPUT=ON \
	-DENABLE_TESTS=ON \
	-DUSE_EXTERN_QUAZIP=ON \
	-GNinja \
	../..

ninja
