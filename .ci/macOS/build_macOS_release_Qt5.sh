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

This script checks neccessary dependencies
and builds a release binary of MediaElch for macOS for Qt5.
Uses hard-coded paths of the current maintainer.

You may need to adapt your \$PATH or macdeployqt may not be found.
  export PATH="\$HOME/Qt/5.15.2/clang_64/bin/:\$PATH"

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

print_important "Using Qt5 from \$HOME/Qt/5.15.2"
export PATH="$HOME/Qt/5.15.2/clang_64/bin/:$OLD_PATH"

# Check for macOS build and packaging dependencies
./.ci/macOS/check_macOS_dependencies.sh

# Update or download in case something is outdated
git submodule update --init

#######################################################
# Download Dependencies

if [[ ! -d "MediaInfoDLL" ]]; then
	print_info "Loading MediaInfoDLL"
	svn checkout https://github.com/MediaArea/MediaInfoLib/trunk/Source/MediaInfoDLL
fi

if [[ ! -d "ZenLib" ]]; then
	print_info "Loading ZenLib"
	svn checkout https://github.com/MediaArea/ZenLib/trunk/Source/ZenLib
fi


#######################################################
# Confirm build

print_system_info_unix

if [[ "${NO_CONFIRM}" != "1" ]]; then
	echo ""
	print_important "Do you want to build MediaElch for macOS with these settings?"
	print_important "The build will take between 5 and 20 minutes dependending on your machine."
	read -r -s -p "Press enter to continue"
	echo ""
fi


#######################################################
# Build Qt5

cd "${PROJECT_DIR}"

print_important "Removing ImageView_Qt6.qml from sources and ui.qrc"
rm -f "${PROJECT_DIR}/src/ui/ImageView_Qt6.qml"
sed '/Qt6/d' "${PROJECT_DIR}/ui.qrc" > ui_qt5.qrc
mv ui_qt5.qrc "${PROJECT_DIR}/ui.qrc"

mkdir -p "${PROJECT_DIR}/build/macOS_Qt5"
cd "${PROJECT_DIR}/build/macOS_Qt5"

# Just in case that it exists. Remove it or macdeployqt may run into issues.
rm -rf MediaElch.app

print_important "Running qmake"
qmake ../../MediaElch.pro CONFIG+=release

print_important "Building MediaElch"
make -j "${JOBS}"

echo ""
print_success "Successfuly built MediaElch for Qt6! Release binary in"
print_success "  ${PROJECT_DIR}/build/macOS_Qt5"
echo ""
