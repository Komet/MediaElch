#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

# Script originally from https://github.com/TheAssassin/AppImageLauncher
# Modified for use in MediaElch

# Does the same as "readlink -f" for *our* use case. "-f" is not available on macOS.
# Requires a trailing slash (/).
function me_readlink() {
	DIR="${1%/*}"
	(cd "$DIR" && pwd -P)
}

cd "$(dirname "$0")"
PROJECT_PATH="$(me_readlink ../../)"

DISTROS=(
	"ubuntu-18.04"
	"ubuntu-20.04"
	"opensuse-leap-42.3"
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

source "${PROJECT_PATH}/scripts/utils.sh"

IMAGE=mediaelch-build:"${DIST}"
DOCKERFILE=Dockerfile.build-"${DIST}"

if [ ! -f "${DOCKERFILE}" ]; then
	echo "Error: ${DOCKERFILE} could not be found"
	echo ""
	print_usage
	exit 1
fi

print_important "Building ${IMAGE} from ${DOCKERFILE} if necessary."
docker build -t "${IMAGE}" -f "${DOCKERFILE}" .

# We use the first part of the distro to distinguish between build scripts
build_sh="build-$(echo ${DIST} | cut -f1 -d-).sh"

print_important "Now building MediaElch using travis-build_release.sh"
print_info "Using user:group id: $(id -u "$(whoami)"):$(id -g "$(whoami)")"

docker run --rm --user "$(id -u "$(whoami)"):$(id -g "$(whoami)")" -it -v ${PROJECT_PATH}:/ws "${IMAGE}" \
	bash -xc "cd /ws && ./.ci/docker/${build_sh} ${DIST}"
