#!/usr/bin/env bash

SCRIPT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"

pushd "$SCRIPT_PATH/.." > /dev/null

echo "Format all files using clang-format"
find . ! -path "*/quazip/*" -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-format -i -style=file {} \;

popd > /dev/null
