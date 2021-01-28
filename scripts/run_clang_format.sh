#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/.." > /dev/null 2>&1

source scripts/utils.sh

if [[ -x "$(command -v clang-format-11)" ]]; then
	CF=clang-format-11
elif [[ -x "$(command -v clang-format-mp-11)" ]]; then
	# MacPorts version
	CF=clang-format-mp-11
else
	CF=clang-format
	clang-format --version | grep " 11." > /dev/null || (print_warning "WARNING: MediaElch requires clang-format version 11")
fi

print_important "Format all source files using ${CF}"
find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec ${CF} -i -style=file {} \+

print_important "Format all test files using ${CF}"
find test -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec ${CF} -i -style=file {} \+

print_success "Done"
