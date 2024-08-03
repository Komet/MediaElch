#!/usr/bin/env bash

# Convenience script that runs all short-running checks.
# Should be run before committing and pushing code.

set -Eeuo pipefail
IFS=$'\n\t'

# Go to scripts directory
cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1

source ./utils.sh

print_important "Run all 5 quick checks"

print_info "\nRunning check 1 / 5 : Clang Format"
./run_clang_format.sh

print_info "\nRunning check 2 / 5 : CMake Format"
./run_cmake_format.sh

print_info "\nRunning check 3 / 5 : Correct debug macros"
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

print_info "\nRunning check 4 / 5 : Shellcheck"
./run_shellcheck.sh

print_info "\nRunning check 5 / 5: Architecture"
if grep -R \
	-e 'QCheckBox' \
	-e 'QWidget' \
	-e 'QGridLayout' \
	-e 'QCheckBox' \
	-e 'QLabel' \
	-e 'small_widgets' \
	../src/scrapers; then
	print_fatal 'Found widget headers in scrapers. Refactor!'
fi

print_success "\nAll checks finished successfully!"
