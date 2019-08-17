#!/usr/bin/env bash

# Convenience script that runs all short-running checks.
# Should be run before committing and pushing code.

set -euo pipefail
IFS=$'\n\t'

cd "$(dirname "$0")"
source utils.sh

print_important "Run all 4 quick checks"

print_info "\nRunning check 1 / 4"
./run_clang_format.sh

print_info "\nRunning check 2 / 4"
./run_cmake_format.sh

print_info "\nRunning check 3 / 4"
./run_shellcheck.sh

print_info "\nRunning check 4 / 4"
./run_cppcheck.sh

print_success "\nAll checks finished successfully!"
