#!/usr/bin/env bash

# Release script for macOS for Qt5.
# Uses hard-coded paths of the current maintainer's system.

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1

#######################################################
# Variables & Functions

PROJECT_DIR="$(pwd -P)"

source .ci/ci_utils.sh

OLD_PATH="$PATH"

if [[ "${OS_NAME}" != "Darwin" ]]; then
	print_fatal "Build script only works on macOS!"
fi

# False positive
# shellcheck disable=SC2154
trap 'rc="$?"; print_error "\\nBuilding MediaElch failed!"; exit ${rc}' SIGINT SIGTERM ERR

usage() {
	cat << EOF
Usage: $(basename "$0") [--no-confirm]

This script checks necessary dependencies
and builds a release binary of MediaElch for macOS for Qt5.
Uses hard-coded paths of the current maintainer.

You may need to adapt your \$PATH or macdeployqt may not be found.
  export PATH="/opt/Qt/${MAC_QT_5_VERSION}/clang_64/bin/:\$PATH"

Options
  --no-confirm   Build MediaElch without confirm dialog.

EOF
	trap - SIGINT SIGTERM ERR
	exit 0
}

parse_params() {
	# default values of variables set from params
	NO_CONFIRM=0

	while :; do
		case "${1-}" in
		-h | --help)
			usage
			;;
		-v | --verbose)
			set -x
			;;
		--no-confirm)
			NO_CONFIRM=1
			;;
		-?*)
			print_fatal "Unknown option: $1"
			;;
		*)
			break
			;;
		esac
		shift
	done

	return 0
}

parse_params "$@"

#######################################################
# Getting Details

export CXX=clang++
export CC=clang

print_important "Using Qt5 from /opt/Qt/${MAC_QT_5_VERSION}"
export PATH="/opt/Qt/${MAC_QT_5_VERSION}/clang_64/bin/:/opt/Qt/${MAC_QT_5_VERSION}/clang_64/:${OLD_PATH}"

# Check for macOS build and packaging dependencies
./.ci/macOS/check_macOS_dependencies.sh

# Update or download in case something is outdated
git submodule update --init

#######################################################
# Download Dependencies

if [[ ! -d "MediaInfoDLL" ]]; then
	print_info "Loading MediaInfoDLL"
	mkdir -p tmp
	rm -rf tmp/MediaInfoDLL # clean up in case previous runs were aborted
	git clone --quiet --depth=1 --branch="v${MAC_MEDIAINFO_VERSION:?}" https://github.com/MediaArea/MediaInfoLib/ tmp/MediaInfoLib
	mv tmp/MediaInfoLib/Source/MediaInfoDLL ./MediaInfoDLL
	rm -rf tmp
fi

if [[ ! -d "ZenLib" ]]; then
	print_info "Loading ZenLib"
	mkdir -p tmp
	rm -rf tmp/ZenLib # clean up in case previous runs were aborted
	git clone --depth=1 --single-branch --branch=master https://github.com/MediaArea/ZenLib/ tmp/ZenLib
	mv tmp/ZenLib/Source/ZenLib ./ZenLib
	rm -rf tmp/ZenLib
fi


#######################################################
# Confirm build

print_system_info_unix

if [[ "${NO_CONFIRM}" != "1" ]]; then
	echo ""
	print_important "Do you want to build MediaElch for macOS with these settings?"
	print_important "The build will take between 5 and 20 minutes depending on your machine."
	read -r -s -p "Press enter to continue"
	echo ""
fi


#######################################################
# Build Qt5

cd "${PROJECT_DIR}"

print_important "Adapting MediaElch.plist"
sed 's/>11</>10.13</' "${PROJECT_DIR}/MediaElch.plist" > MediaElch_qt5.plist
mv MediaElch_qt5.plist "${PROJECT_DIR}/MediaElch.plist"

mkdir -p "${PROJECT_DIR}/build/macOS_Qt5"
cd "${PROJECT_DIR}/build/macOS_Qt5"

# Just in case that it exists. Remove it or macdeployqt may run into issues.
rm -rf MediaElch.app

print_important "Running CMake"
cmake -S ../.. -B . \
	-DCMAKE_BUILD_TYPE=Release \
	-DENABLE_COLOR_OUTPUT=ON  \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DENABLE_LTO=ON \
	-DMEDIAELCH_FORCE_QT5=ON \
	-GNinja

print_important "Building MediaElch"
ninja

echo ""
print_success "Successfully built MediaElch for Qt6! Release binary in"
print_success "  ${PROJECT_DIR}/build/macOS_Qt5"
echo ""
