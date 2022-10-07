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

# TODO: performance + enable some suppressed checks
# Notes: ignore quazip, use qt.cfg
cppcheck --enable=style,warning,portability,information,missingInclude \
	--error-exitcode=1 \
	--inline-suppr \
	-q \
	-Isrc \
	-j4 \
	--suppress=preprocessorErrorDirective \
	--suppress=ignoredReturnValue \
    --suppress=ignoredReturnErrorCode \
    --suppress=useStlAlgorithm \
    --suppress=useInitializationList \
    --suppress=qSortCalled \
    --suppress=constParameter \
    --suppress=constVariable \
    --suppress=passedByValue \
    --suppress=noExplicitConstructor \
    --suppress=shadowFunction \
    --suppress=unreadVariable \
    --suppress=knownConditionTrueFalse \
    --suppress=duplicateBreak \
    --suppress=unsafeClassCanLeak \
    --suppress=duplicateCondition \
    --suppress=redundantInitialization \
    --suppress=cstyleCast \
    --suppress=qSwapCalled \
    --suppress=variableScope \
    --suppress=unusedVariable \
    --suppress=constParameterCallback \
	--inline-suppr \
	--std=c++14 \
	--library=qt \
	--project=./build/ci/compile_commands.json \
	-ithird_party/quazip

print_success "No issues found! Great! :-)"
