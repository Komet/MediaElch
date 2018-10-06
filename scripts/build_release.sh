#!/usr/bin/env bash

set -e

export SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
export PROJECT_DIR="$( cd "${SCRIPT_DIR}/.." ; pwd -P )"
export BUILD_DIR="${PROJECT_DIR}/build"
BUILD_OS=$1

cd "${SCRIPT_DIR}"
source utils.sh
source build-scripts/check_dependencies.sh
source build-scripts/build_functions.sh

if [ ! -f "/etc/debian_version" ]; then
	print_critical "Build script only works on Debian/Ubuntu systems!"
fi

print_help() {
	echo ""
	echo "Usage: ./build_release.sh [linux|win] [options]"
	echo ""
	echo "This script checks neccessary dependencies"
	echo "and builds a release binary of MediaElch."
	echo ""
	echo "Options"
	echo "  --no-confirm   Build MediaElch without confirm dialog."
	echo ""
}

confirm_build() {
	echo ""
	print_important "Do you want to build MediaElch for ${BUILD_OS} with these settings?"
	print_important "The build will take between 5 and 20 minutes dependending on your machine."
	read -s -p  "Press enter to continue"
	echo ""
	echo ""
}

build_release_linux() {
	if [ "${BUILD_DIR}" == "" ]; then
		echo "\$BUILD_DIR not set."
		exit 1
	fi

	rm -rf "${BUILD_DIR}"
	mkdir -p "${BUILD_DIR}"

	pushd ${BUILD_DIR} > /dev/null
	print_important "Running qmake"
	qmake "${PROJECT_DIR}/MediaElch.pro" CONFIG+=release
	echo ""

	print_important "Building MediaElch (only warnings and errors shown)"
	make -j $(nproc) 1>/dev/null
	popd > /dev/null
}


if [ "${BUILD_OS}" == "linux" ]; then
	check_dependencies_linux
	print_system_info_linux
	[ "${2}" != "--no-confirm" ] && confirm_build
	build_release_linux || {
		print_critical "Build failed!"
	}

elif [ "${BUILD_OS}" == "win" ]; then
	print_info "Windows build not supported, yet!"
	exit 1

else
	print_error "Unknown OS \"${BUILD_OS}\""
	print_help
	exit 1
fi

echo ""
print_success "Successfuly built MediaElch! Release binary in \"${BUILD_DIR}\"."
echo ""
