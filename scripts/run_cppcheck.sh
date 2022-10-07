#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

###############################################################################
# Run cppcheck on all sources (for usage in CIs).
###############################################################################

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/.." > /dev/null 2>&1

source scripts/utils.sh

print_important "Run cppcheck on all source files"

if [[ ! -f ./build/ci/compile_commands.json ]]; then
	echo ""
	print_important "compile_command.json not found, running 'cmake --preset ci' and building it for Qt moc-files"
	cmake --preset ci
	cmake --build --preset ci
fi

print_info "Will only print cppcheck warnings"

# TODO: performance
# Notes: ignore quazip, use qt.cfg
cppcheck --enable=warning,portability,information,missingInclude \
	--error-exitcode=1 \
	--inline-suppr \
	-q \
	-Isrc \
	-j4 \
	--suppress=preprocessorErrorDirective \
	--suppress=ignoredReturnValue \
	--inline-suppr \
	--std=c++14 \
	--library=qt \
	--project=./build/ci/compile_commands.json \
	-ithird_party/quazip

print_success "No issues found! Great! :-)"
