#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1
PROJECT_DIR="$(pwd -P)"

ME_UID=$(id -u "$(whoami)")
ME_GUID="$(id -g "$(whoami)")"

# Qt6 -----------------------------------------------------

ME_DOCKER_IMAGE_MXE="mediaelch/mediaelch-ci-win:qt6"

# Note: Running "ls -la" on the volume avoids some strange
# permission bug, where /src/mediaelch is owned by root otherwise.
docker run --rm --user "${ME_UID}:${ME_GUID}" \
	-v "${PROJECT_DIR}:/src/mediaelch" "${ME_DOCKER_IMAGE_MXE}" \
	/bin/bash -xc "ls -la /src/mediaelch && cd /src/mediaelch && QT_MAJOR_VERSION=6 ./.ci/win/build_windows_release.sh --no-confirm"
