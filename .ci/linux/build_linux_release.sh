#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1

#######################################################
# Variables & Functions

PROJECT_DIR="$(pwd -P)"

source .ci/ci_utils.sh

if [[ ! -f "/etc/debian_version" ]]; then
	print_fatal "Package script only works on Debian/Ubuntu systems!"
fi

# False positive
# shellcheck disable=SC2154
trap 'rc="$?"; print_error "\\nBuilding MediaElch failed!"; exit ${rc}' SIGINT SIGTERM ERR

usage() {
	cat << EOF
Usage: $(basename "$0") [--no-confirm]

This script checks neccessary dependencies
and builds a release binary of MediaElch for Linux.

You may need to adapt your \$PATH or macdeployqt may not be found.
  export PATH="\$HOME/Qt/5.15.2/gcc_64/bin/:\$PATH"
	  or
	source /opt/qt512/bin/qt512-env.sh

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

# Check for linux build and packaging dependencies
source ./.ci/linux/check_linux_dependencies.sh
check_dependencies_linux

# Update or download in case something is outdated
git submodule update --init

print_system_info_unix

if [[ "${NO_CONFIRM}" != "1" ]]; then
	echo ""
	print_important "Do you want to build MediaElch for Linux with these settings?"
	print_important "The build will take between 5 and 20 minutes dependending on your machine."
	read -r -s -p "Press enter to continue"
	echo ""
fi

#######################################################
# Build

cd "${PROJECT_DIR}"
mkdir -p build/linux
cd build/linux

print_important "Running qmake"
qmake ../../MediaElch.pro CONFIG+=release

print_important "Building MediaElch"
make -j "${JOBS}"

echo ""
print_success "Successfuly built MediaElch! Release binary in"
print_success "  ${PROJECT_DIR}/build/linux"
echo ""
