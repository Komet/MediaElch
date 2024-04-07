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

if command -v "qmake-qt5" > /dev/null 2>&1; then
  qmake-qt5 --version
else
  cmake --version
fi

#################################################
# Build MediaElch
#################################################

print_info "Loading submodules"
git submodule update --init

print_info "Build directory: $(readlink -f build/build-${DIST})"
rm -rf build/build-${DIST}
mkdir -p build/build-${DIST} && cd $_

if command -v "qmake-qt5" > /dev/null 2>&1; then
  print_important "Running qmake"
  qmake-qt5 ../../MediaElch.pro CONFIG+=release
else
  print_important "Running cmake"
  cmake ../.. -DCMAKE_BUILD_TYPE=Release -DMEDIAELCH_FORCE_QT6=ON
fi

echo ""

print_important "Building MediaElch (only warnings and errors shown)"
make -j "$(nproc)" 1> /dev/null

echo ""
print_success "Successfuly built MediaElch on ${DIST}!"
echo ""
