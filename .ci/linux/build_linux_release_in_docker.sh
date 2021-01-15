#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1
PROJECT_DIR="$(pwd -P)"

# Build Docker Image if necessary
cd ./.ci/docker
# 16.04, oldest system we support with AppImages
docker build -t "mediaelch-build:ubuntu-16.04" -f "Dockerfile.build-ubuntu-16.04" .
cd ../..

ME_UID=$(id -u "$(whoami)")
ME_GUID="$(id -g "$(whoami)")"

docker run --rm --user "${ME_UID}:${ME_GUID}" \
	-v "${PROJECT_DIR}:/src" "mediaelch-build:ubuntu-16.04" \
	/bin/bash -xc "source /opt/qt512/bin/qt512-env.sh && cd /src && ./.ci/linux/build_linux_release.sh --no-confirm"
