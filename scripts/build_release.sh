#!/usr/bin/env bash

set -e          # Exit on errors
set -o pipefail # Unveils hidden failures

SCRIPT_DIR="$(
	cd "$(dirname "$0")"
	pwd -P
)"
PROJECT_DIR="$(
	cd "${SCRIPT_DIR}/.."
	pwd -P
)"
BUILD_DIR="${PROJECT_DIR}/build"
BUILD_OS=$1

cd "${SCRIPT_DIR}"
source utils.sh
source packaging/check_dependencies.sh

if [[ ! -f "/etc/debian_version" ]] && [[ "${OS_NAME}" != "Darwin" ]]; then
	print_fatal "Build script only works on Debian/Ubuntu systems and macOS!"
	exit 1
fi

print_help() {
	echo ""
	echo "Usage: ./build_release.sh [linux|win|macOS] [options]"
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
	read -r -s -p "Press enter to continue"
	echo ""
	echo ""
}

build_failed() {
	print_error "Build failed!"
	exit 1
}

build_release_unix() {
	if [ "${BUILD_DIR}" == "" ]; then
		echo "\$BUILD_DIR not set."
		exit 1
	fi

	rm -rf "${BUILD_DIR}"
	mkdir -p "${BUILD_DIR}"

	pushd "${BUILD_DIR}" > /dev/null
	print_important "Running qmake"
	qmake "${PROJECT_DIR}/MediaElch.pro" CONFIG+=release || build_failed
	echo ""

	print_important "Building MediaElch (only warnings and errors shown)"
	make -j "${JOBS}" 1> /dev/null || build_failed
	popd > /dev/null
}

if [ "${BUILD_OS}" == "linux" ]; then
	check_dependencies_linux
	print_system_info_unix
	[ "${2}" != "--no-confirm" ] && confirm_build
	build_release_unix || {
		print_fatal "Build failed!"
		exit 1
	}

elif [ "${BUILD_OS}" == "win" ]; then
	print_info "Windows build not supported, yet!"
	exit 1

elif [ "${BUILD_OS}" == "macOS" ]; then

	pushd "${PROJECT_DIR}" > /dev/null
	if [[ ! -d "MediaInfoDLL" ]]; then
		print_info "Loading MediaInfoDLL"
		svn checkout https://github.com/MediaArea/MediaInfoLib/trunk/Source/MediaInfoDLL
	fi
	if [[ ! -d "ZenLib" ]]; then
		print_info "Loading ZenLib"
		svn checkout https://github.com/MediaArea/ZenLib/trunk/Source/ZenLib
	fi
	popd > /dev/null

	check_dependencies_macOS
	print_system_info_unix
	[ "${2}" != "--no-confirm" ] && confirm_build
	build_release_unix || {
		print_fatal "Build failed!"
		exit 1
	}

else
	print_error "Unknown OS \"${BUILD_OS}\""
	print_help
	exit 1
fi

echo ""
print_success "Successfuly built MediaElch! Release binary in \"${BUILD_DIR}\"."
echo ""
