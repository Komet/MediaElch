#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

###############################################################################
# Run cppcheck on all sources (for usage in CIs)
# If you develop for this project, please use `make cppcheck`.
###############################################################################

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/.." > /dev/null 2>&1

source scripts/utils.sh

print_important "Run cppcheck on all source files"
print_info "Will only print warnings"

cppcheck --enable=performance,warning,portability,information,missingInclude \
	--error-exitcode=1 \
	--inline-suppr \
	-q \
	-Isrc \
	-j2 \
	./src

print_success "No issues found! Great! :-)"
