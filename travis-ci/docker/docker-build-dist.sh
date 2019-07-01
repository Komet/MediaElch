#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

# Script originally from https://github.com/TheAssassin/AppImageLauncher
# Modified for use in MediaElch

cd $(dirname "$(realpath -s "$0")")
PROJECT_PATH="$(readlink -f ../..)"

DISTROS=(
	"ubuntu-16.04"
	"ubuntu-18.04"
	"ubuntu-19.04"
	"opensuse-leap-42.3"
	"opensuse-leap-15"
	"opensuse-tumbleweed"
)

print_usage() {
	echo "Usage:"
	echo "  $0 <distro>"
	echo ""
	echo "Distros:"
	for distro in ${DISTROS[@]}; do
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
docker run --rm -it -v ${PROJECT_PATH}:/ws "${IMAGE}" \
	bash -xc "cd /ws && ./travis-ci/docker/${build_sh} ${DIST}"
