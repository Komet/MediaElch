#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1

./.ci/macOS/build_macOS_release_Qt5.sh --no-confirm && ./.ci/macOS/package_macOS_Qt5.sh --no-confirm
./.ci/macOS/build_macOS_release_Qt6.sh --no-confirm && ./.ci/macOS/package_macOS_Qt6.sh --no-confirm
