#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

# Script originally from https://github.com/TheAssassin/AppImageLauncher
# Modified for use in MediaElch

# Does the same as "readlink -f" for *our* use case. "-f" is not available on macOS.
# Requires a trailing slash (/).
function make_absolute_path() {
	DIR="${1%/*}"
	(cd "$DIR" && pwd -P)
}

cd "$(dirname "$0")"
PROJECT_PATH="$(make_absolute_path ../../)"

DISTROS=(
	"ubuntu-16.04"
	"ubuntu-20.04"
	"ubuntu-22.04"
	"ubuntu-24.04"
	"opensuse-leap-15"
	"opensuse-tumbleweed"
)

print_usage() {
	echo "Usage:"
	echo "  $0 <distro>"
	echo ""
	echo "Distros:"
	for distro in "${DISTROS[@]}"; do
		echo " - ${distro}"
	done
	echo ""
}

if [ -z ${1+x} ]; then
	echo "Error: No distribution set."
	echo ""
	print_usage
	exit 1
fi

DIST="${1}"
# We use the first part of the distro to distinguish between build scripts (Ubuntu/openSUSE)
OS="$(echo "${DIST}" | cut -f1 -d-)"

source "${PROJECT_PATH}/scripts/utils.sh"

IMAGE=mediaelch/mediaelch-build:"${DIST}"
DOCKERFILE=Dockerfile.build-"${DIST}"

if [ ! -f "${DOCKERFILE}" ]; then
	echo "Error: ${DOCKERFILE} could not be found"
	echo ""
	print_usage
	exit 1
fi

print_important "Building ${IMAGE} from ${DOCKERFILE} if necessary."
docker build --pull -t "${IMAGE}" -f "${DOCKERFILE}" .

BUILD_SCRIPT="./.ci/docker/build-${OS:?}.sh"
print_important "Now building MediaElch using ${BUILD_SCRIPT}"
print_info "Using user:group id: $(id -u "$(whoami)"):$(id -g "$(whoami)")"

if docker version |& grep podman >/dev/null; then
  docker run --rm -it -v "${PROJECT_PATH}":/opt/src "${IMAGE}" \
  	bash -xc "cd /opt/src && ${BUILD_SCRIPT} ${DIST}"
else
  docker run --rm --user "$(id -u "$(whoami)"):$(id -g "$(whoami)")" -it -v "${PROJECT_PATH}":/opt/src "${IMAGE}" \
  	bash -xc "cd /opt/src && ${BUILD_SCRIPT} ${DIST}"
fi