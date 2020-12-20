#!/usr/bin/env bash

###########################################################
# This script is intended to be used by a docker container.
# It may work standalone but it is not tested.
###########################################################

set -euo pipefail
IFS=$'\n\t'

if [ -z ${1+x} ]; then
	echo "Error: No distribution set."
	echo "For usage see .ci/docker/build-docker-dist.sh"
	exit 1
fi

cd "$(dirname "$(realpath -s "$0")")/../.."
DIST="${1}"

# Utils for printing, colors, etc.
source ./scripts/utils.sh

#################################################
# System Information
#################################################

echo ""
print_important "System Information"
echo "  Architecture: $(uname -r)"
echo "  Machine:      $(uname -m)"
echo ""

g++ --version
qmake-qt5 --version

#################################################
# Build MediaElch
#################################################

print_info "Loading submodules"
git submodule update --init

print_info "Build directory: $(readlink -f build-${DIST})"
rm -rf build-${DIST}
mkdir -p build/build-${DIST} && cd $_

print_important "Running qmake"
qmake-qt5 ../../MediaElch.pro CONFIG+=release
echo ""

print_important "Building MediaElch (only warnings and errors shown)"
make -j "$(nproc)" 1> /dev/null

echo ""
print_success "Successfuly built MediaElch on ${DIST}!"
echo ""
