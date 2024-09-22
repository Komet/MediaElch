#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1
PROJECT_DIR="$(pwd -P)"

ME_DOCKER_IMAGE_MXE="mediaelch/mediaelch-ci-win:qt5"
ME_UID=$(id -u "$(whoami)")
ME_GUID="$(id -g "$(whoami)")"

if docker version |& grep podman >/dev/null; then
  docker run --rm --user ":" \
    -v "${PROJECT_DIR:?}:/src/mediaelch" "${ME_DOCKER_IMAGE_MXE:?}" \
    /bin/bash -xc "git config --global --add safe.directory '/src/' && ls -la /src/mediaelch && cd /src/mediaelch && QT_MAJOR_VERSION=5 ./.ci/win/package_windows.sh"
else
  docker run --rm --user "${ME_UID:?}:${ME_GUID:?}" \
    -v "${PROJECT_DIR:?}:/src/mediaelch" "${ME_DOCKER_IMAGE_MXE:?}" \
    /bin/bash -xc "git config --global --add safe.directory '/src/' && ls -la /src/mediaelch && cd /src/mediaelch && QT_MAJOR_VERSION=5 ./.ci/win/package_windows.sh"
fi