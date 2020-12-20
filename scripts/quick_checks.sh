#!/usr/bin/env bash

# Convenience script that runs all short-running checks.
# Should be run before committing and pushing code.

set -Eeuo pipefail
IFS=$'\n\t'

# Go to scripts directory
cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1

source ./utils.sh

print_important "Run all 4 quick checks"

print_info "\nRunning check 1 / 4"
./run_clang_format.sh

print_info "\nRunning check 2 / 4"
./run_cmake_format.sh

print_info "\nRunning check 3 / 4"
./run_shellcheck.sh

print_info "\nRunning check 4 / 4"
print_important "Running shfmt"
find . -type f -iname '*.sh' -exec shfmt -l -w {} \+

print_success "\nAll checks finished successfully!"
