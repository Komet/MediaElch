#!/usr/bin/env bash

set -e

SCRIPT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"

pushd "${SCRIPT_PATH}/.." > /dev/null

echo "Run cppcheck on all source files"

cppcheck --enable=all --error-exitcode=1 -Isrc -j2 ./src

popd > /dev/null
