#!/usr/bin/env bash

set -e          # Exit on errors
set -o pipefail # Unveils hidden failures

export SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
export PROJECT_DIR="$( cd "${SCRIPT_DIR}/.." ; pwd -P )"
export BUILD_DIR="${PROJECT_DIR}/build"
BUILD_OS=${1:-}
PACKAGE_TYPE=${2:-}

cd "${SCRIPT_DIR}"
source utils.sh
source build_scripts/check_dependencies.sh
source build_scripts/package_appimage.sh
source build_scripts/package_deb.sh

if [ ! -f "/etc/debian_version" ]; then
	print_critical "Package script only works on Debian/Ubuntu systems!"
fi

print_help() {
	echo ""
	echo "Usage: ./package_release.sh (linux|win) package-type [options]"
	echo ""
	echo "This script builds and packages MediaElch in a distributable form."
	echo "It is used by MediaElch authors to release official MediaElch releases."
	echo "Use build_release.sh if you want to build MediaElch."
	echo ""
	echo "Package Types:"
	echo "  linux"
	echo "    AppImage  Create a linux app image. See https://appimage.org/"
	echo "    deb       Build an unsigned deb package."
	echo "    launchpad Upload a new MediaElch version to launchpad.net. You need a "
	echo "              clean repository for that (no build files)."
	echo "  win"
	echo "    zip       Create a ZIP file containing MediaElch and its dependencies."
	echo ""
	echo "Options"
	echo "  --no-confirm   Package MediaElch without confirm dialog."
	echo ""
}

# Gather information for packaging MediaElch such as version, git
# hash and date.
gather_information() {
	pushd "${PROJECT_DIR}" > /dev/null

	ME_VERSION=$(sed -ne 's/.*AppVersionStr[^"]*"\(.*\)";/\1/p' Version.h)
	GIT_DATE=$(git --git-dir=".git" show --no-patch --pretty="%ci")
	#RELEASE_DATE=$(date -u +"%Y-%m-%dT%H:%M:%S%z" --date="${GIT_DATE}")
	GIT_HASH=$(git --git-dir=".git" show --no-patch --pretty="%h")
	DATE_HASH=$(date -u +"%Y-%m-%d_%H-%M")
	VERSION_NAME="${ME_VERSION}_${DATE_HASH}_git-${TRAVIS_BRANCH}-${GIT_HASH}"

	print_important "Information used for packaging:"
	echo "  Git Date          = ${GIT_DATE}"
	#echo "  Release Date      = ${RELEASE_DATE}"
	echo "  Git Hash          = ${GIT_HASH}"
	echo "  Date Hash         = ${DATE_HASH}"
	echo "  MediaElch Version = ${ME_VERSION}"
	echo "  Version Name      = ${VERSION_NAME}"

	popd > /dev/null
}

confirm_build() {
	echo ""
	print_important "Do you want to package MediaElch for ${BUILD_OS} with these settings?"
	read -s -p  "Press enter to continue"
	echo ""
}

pkg_type="$(lc ${PACKAGE_TYPE:-invalid})"
no_confirm=${3:-confirm}

if [ "${BUILD_OS}" == "linux" ] ; then
	if [ $pkg_type != "appimage" ] && [ $pkg_type != "deb" ]; then
		print_error "Unknown package type for linux: \"${PACKAGE_TYPE}\""
		print_help
		exit 1
	fi

	gather_information
	[ "${no_confirm}" != "--no-confirm" ] && confirm_build
	echo ""

	if [ $pkg_type == "appimage" ]; then
		package_appimage ${no_confirm}
	elif [ $pkg_type == "launchpad" ]; then
		package_and_upload_to_launchpad
	elif [ $pkg_type == "deb" ]; then
		package_deb
	fi

elif [ "${BUILD_OS}" == "win" ]; then
	print_info "Windows build not supported, yet!"
	exit 1

else
	print_error "Unknown OS \"${BUILD_OS}\""
	print_help
	exit 1
fi

