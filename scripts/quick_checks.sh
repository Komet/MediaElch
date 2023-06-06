#!/usr/bin/env bash

# Convenience script that runs all short-running checks.
# Should be run before committing and pushing code.

set -Eeuo pipefail
IFS=$'\n\t'

# Go to scripts directory
cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1

source ./utils.sh

print_important "Run all 5 quick checks"

print_info "\nRunning check 1 / 4 : Clang Format"
./run_clang_format.sh

print_info "\nRunning check 2 / 4 : CMake Format"
./run_cmake_format.sh

print_info "\nRunning check 3 / 4 : Correct debug macros"
if grep -r 'qInfo' ../src; then
	print_fatal "Found usages of qInfo! Use qCInfo(category) instead!"
elif grep -r 'qDebug' ../src; then
	print_fatal "Found usages of qDebug! Use qCDebug(category) instead!"
elif grep -r 'qWarning' ../src; then
	print_fatal "Found usages of qWarning! Use qCWarning(category) instead!"
elif grep -r 'qCritical' ../src; then
	print_fatal "Found usages of qCritical! Use qCCritical(category) instead!"
else
	print_success "Done."
fi

print_info "\nRunning check 4 / 4 : Shellcheck"
./run_shellcheck.sh

print_success "\nAll checks finished successfully!"
