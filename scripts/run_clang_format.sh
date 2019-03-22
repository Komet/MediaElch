#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

SCRIPT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"

cd "$SCRIPT_PATH/.."

echo "Format all source files using clang-format"
find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i -style=file {} \;

echo "Format all test files using clang-format"
find test -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i -style=file {} \;
