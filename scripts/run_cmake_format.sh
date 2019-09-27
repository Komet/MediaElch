#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

###############################################################################
# Run cmake-format on all CMake files (for usage in CIs)
# If you develop for this project, please use `make cmake-format`.
###############################################################################

cd "$(dirname "$0")/.."
source scripts/utils.sh

print_important "Run cmake-format on all CMake files"

find . ! -path "./build/*" ! -path "./third_party/*" \
    -type f \( -name "*.cmake" -o -name "CMakeLists.txt" \) \
    -exec cmake-format -c .cmake-format -i {} \+

cmake-format -c .cmake-format -i third_party/CMakeLists.txt

print_success "Formatted all CMake files! Great! :-)"
