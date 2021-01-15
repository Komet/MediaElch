#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1
PROJECT_DIR="$(pwd -P)"

ME_UID=$(id -u "$(whoami)")
ME_GUID="$(id -g "$(whoami)")"

docker run --rm --user "${ME_UID}:${ME_GUID}" \
	-v "${PROJECT_DIR}:/src" "mediaelch-build:ubuntu-16.04" \
	/bin/bash -xc "source /opt/qt512/bin/qt512-env.sh && cd /src && ./.ci/linux/package_linux_appimage.sh --no-confirm"
