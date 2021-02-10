#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

###############################################################################
# Run shellcheck on all bash script (for usage in CIs)
# This is a convenience script that excludes some warnings.
###############################################################################

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/.." > /dev/null 2>&1

source scripts/utils.sh

print_important "Run shellcheck on all source files"

find . ! -path "./build/*" ! -path "./third_party/*" -type f -name "*.sh" \
	-exec shellcheck --external-sources --color=always \
	-e SC2086,SC1090,SC2143,SC1091,SC2010,SC2103 \
	{} \+

# SC2086: Double quote to prevent globbing and word splitting.
# SC1090: Can't follow non-constant source. Use a directive to specify location
# SC2143: Use grep -q instead of comparing output with
# SC1091: Not following
# SC2010: Don't use ls | grep. Use a glob or a for loop with a condition to allow non-alphanumeric filenames.
# SC2103: Use a ( subshell ) to avoid having to cd back.

print_success "No issues found! Great! :-)"
