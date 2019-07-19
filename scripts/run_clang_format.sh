#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

cd "$(dirname "$0")/.."
source scripts/utils.sh

clang-format --version | grep " 7." > /dev/null || ( print_warning "WARNING: MediaElch requires clang-format version 7")

print_info "Format all source files using clang-format"
find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i -style=file {} \+

print_info "Format all test files using clang-format"
find test -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i -style=file {} \+

print_success "Done"
