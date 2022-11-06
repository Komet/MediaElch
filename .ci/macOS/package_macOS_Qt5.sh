#!/usr/bin/env bash

# Package script for macOS for Qt5.
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
	print_fatal "Packaging script only works on macOS!"
fi

usage() {
	cat << EOF
Usage: $(basename "$0") [--no-confirm]

This script builds and packages MediaElch in a distributable form for macOS.
It is used by MediaElch authors to release official MediaElch releases and
for our nightly builds.

    DO NOT USE THIS IF YOU'RE AN END-USER
    UNLESS YOU KNOW WHAT YOU'RE DOING!

Use .ci/macOS/build_macOS_release_Qt5.sh if you want to build MediaElch.

You may need to adapt your \$PATH or macdeployqt may not be found.
  export PATH="\$HOME/Qt/5.15.2/clang_64/bin/:\$PATH"

Options
  --no-confirm   Package MediaElch without confirm dialog.
EOF
	exit
}

confirm_build() {
	echo ""
	print_important "Do you want to package MediaElch ${ME_VERSION} for macOS with these settings?"
	print_important "It is recommended to clean your repository using \"git clean -fdx\" first."
	read -r -s -p "Press enter to continue, Ctrl+C to cancel"
	echo ""
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

print_important "Using Qt5 from \$HOME/Qt/5.15.2"
export PATH="$HOME/Qt/5.15.2/clang_64/bin/:$OLD_PATH"

# Check for macOS build and packaging dependencies
./.ci/macOS/check_macOS_dependencies.sh

# Exports required variables
gather_project_and_system_details

if [[ "${NO_CONFIRM}" != "1" ]]; then
	confirm_build
fi
echo ""

#######################################################
# Check that MediaElch is build

if [ ! -d "./build/macOS_Qt5" ] || [ ! -f "./build/macOS_Qt5/.qmake.stash" ] || [ ! -d "./build/macOS_Qt5/MediaElch.app/Contents/MacOS" ]; then
	print_fatal "Build MediaElch with Qt5 before packaging it!"
fi

#######################################################
# Download Dependencies into third_party folder

mkdir -p "${PROJECT_DIR}/third_party/packaging_macOS_Qt5"
cd "${PROJECT_DIR}/third_party/packaging_macOS_Qt5"

#######################################################
# MediaInfoDLL

if [[ ! -f libmediainfo.0.dylib ]]; then
	print_info "Downloading libmediainfo"

	wget --output-document MediaInfo_DLL.tar.bz2 "${MAC_MEDIAINFO_URL}"
	validate_sha512 "MediaInfo_DLL.tar.bz2" "${MAC_MEDIAINFO_SHA512}"

	tar -xvjf MediaInfo_DLL.tar.bz2
	mv MediaInfoLib/libmediainfo.0.dylib ./
	rm -rf MediaInfoLib
	rm MediaInfo_DLL.tar.bz2
fi

#######################################################
# Install create-dmg

if [[ ! -d create-dmg ]]; then
	print_info "Downloading create-dmg"
	git clone "${MAC_CREATE_DMG_GIT_REPO}"
	pushd "create-dmg" > /dev/null
	git checkout "${MAC_CREATE_DMG_GIT_HASH}" > /dev/null # hard-coded version
	popd > /dev/null
fi

#######################################################
# ffmpeg

if [[ ! -f ffmpeg ]]; then
	print_info "Downloading ffmpeg"
	wget --output-document ffmpeg.7z "${MAC_FFMPEG_URL}"
	validate_sha512 "ffmpeg.7z" "${MAC_FFMPEG_SHA512}"
	7za e ffmpeg.7z
	rm ffmpeg.7z
fi

#######################################################
# Copying dependencies

cd "${PROJECT_DIR}/build/macOS_Qt5"

# Remove existing *.dmg files created by an old run of macdeployqt.
rm -f ./*.dmg

cp ../../third_party/packaging_macOS_Qt5/ffmpeg MediaElch.app/Contents/MacOS/
cp ../../third_party/packaging_macOS_Qt5/libmediainfo.0.dylib MediaElch.app/Contents/MacOS/

#########################################
# Translations

QT_TRANSLATIONS_PATH="$(qmake -query QT_INSTALL_TRANSLATIONS)"
print_info "Copying Qt's translations from ${QT_TRANSLATIONS_PATH}"
mkdir -p MediaElch.app/Contents/translations
# Note: Even qtscript_XX.qm is required. *.qm files seem to depend on each other.
cp "${QT_TRANSLATIONS_PATH}"/qt*.qm MediaElch.app/Contents/translations/

# Check that translations were copied by looking for German
if [[ ! -f MediaElch.app/Contents/translations/qt_de.qm ]]; then
	print_fatal "German translation for Qt is missing!"
fi

#######################################################
# Packaging into DMG

print_info "Running macdeployqt"
macdeployqt MediaElch.app -qmldir="${PROJECT_DIR}/src/ui" -verbose=2

print_info "Running create-dmg"
# Note: Icon/Image path needs to be absolute
../../third_party/packaging_macOS_Qt5/create-dmg/create-dmg \
	--volname "MediaElch" \
	--volicon "${PROJECT_DIR}/MediaElch.icns" \
	--background "${PROJECT_DIR}/.ci/macOS/backgroundImage.tiff" \
	--window-pos 200 120 \
	--window-size 550 400 \
	--icon-size 100 \
	--icon MediaElch.app 150 190 \
	--hide-extension MediaElch.app \
	--app-drop-link 400 190 \
	"MediaElch_macOS_Qt5_${ME_VERSION_NAME}.dmg" \
	MediaElch.app

# Move *.dmg into root directory
mv "MediaElch_macOS_Qt5_${ME_VERSION_NAME}.dmg" ../..

print_success "Successfully packaged MediaElch for macOS with Qt5"
print_success ".dmg can be found at:"
print_success "  $(pwd)/MediaElch_macOS_Qt5_${ME_VERSION_NAME}.dmg"
echo ""
