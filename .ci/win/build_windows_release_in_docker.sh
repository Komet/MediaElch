#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1
PROJECT_DIR="$(pwd -P)"

ME_DOCKER_IMAGE_MXE="mediaelch/mediaelch-ci-win:latest"
ME_UID=$(id -u "$(whoami)")
ME_GUID="$(id -g "$(whoami)")"

docker run --rm --user "${ME_UID}:${ME_GUID}" \
	-v "${PROJECT_DIR}:/src" "${ME_DOCKER_IMAGE_MXE}" \
	/bin/bash -xc "cd /src && ./.ci/win/build_windows_release.sh --no-confirm"
