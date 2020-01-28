#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

cd "$(dirname "$0")/.."
source scripts/utils.sh

if [[ -x "$(command -v clang-format-9)" ]]; then
	CF=clang-format-9
else
	CF=clang-format
	clang-format --version | grep " 9." > /dev/null || ( print_warning "WARNING: MediaElch requires clang-format version 9")
fi

print_important "Format all source files using ${CF}"
find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec ${CF} -i -style=file {} \+

print_important "Format all test files using ${CF}"
find test -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec ${CF} -i -style=file {} \+

print_success "Done"
