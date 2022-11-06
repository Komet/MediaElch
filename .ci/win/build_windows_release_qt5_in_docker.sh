#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1
PROJECT_DIR="$(pwd -P)"

ME_UID=$(id -u "$(whoami)")
ME_GUID="$(id -g "$(whoami)")"

# Qt5 -----------------------------------------------------

ME_DOCKER_IMAGE_MXE="mediaelch/mediaelch-ci-win:qt5"

docker run --rm --user "${ME_UID}:${ME_GUID}" \
	-v "${PROJECT_DIR}:/src" "${ME_DOCKER_IMAGE_MXE}" \
	/bin/bash -xc "cd /src && QT_MAJOR_VERSION=5 ./.ci/win/build_windows_release.sh --no-confirm"
