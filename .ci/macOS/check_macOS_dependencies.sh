#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'
HAS_MISSING=0

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1

source ./.ci/ci_utils.sh

print_important "Checking dependencies for macOS:"
echo ""

require_command git
require_command qmake
require_command clang++
require_command svn
require_command tar
require_command wget
require_command 7za
require_command macdeployqt

echo ""

if [ ${HAS_MISSING} -eq 0 ]; then
	print_success "All dependencies installed."
else
	print_fatal "Found missing dependencies. Abort."
fi
