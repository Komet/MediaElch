#!/usr/bin/env bash

set -e          # Exit on errors
set -o pipefail # Unveils hidden failures
set -u          # No unset variables
IFS=$'\n\t'

SCRIPT_DIR="$(
	cd "$(dirname "$0")"
	pwd -P
)"
PROJECT_DIR="$(
	cd "${SCRIPT_DIR}/../.."
	pwd -P
)"
BUILD_DIR="${PROJECT_DIR}/build"

export MEDIAINFO_VERSION=20.09
export FFMPEG_VERSION=4.3.1

cd "${SCRIPT_DIR}"
source ../utils.sh
source ./package_utils.sh
source check_dependencies.sh

if [[ "$OS_NAME" != "Darwin" ]]; then
	print_critical "Package script only works on macOS!"
fi

print_help() {
	echo ""
	echo "Usage: ./package_macOS.sh [options]"
	echo ""
	echo "This script builds and packages MediaElch in a distributable form for macOS."
	echo "It is used by MediaElch authors to release official MediaElch releases."
	echo ""
	echo "    DO NOT USE THIS IF YOU'RE AN END-USER"
	echo ""
	echo "Use build_release.sh if you want to build MediaElch."
	echo ""
	echo "You may need to adapt your \$PATH:"
	echo "  export PATH=\"\$HOME/Qt/5.15.1/clang_64/bin/:\$PATH\""
	echo ""
	echo "Options"
	echo "  --no-confirm   Package MediaElch without confirm dialog."
	echo ""
}

confirm_build() {
	echo ""
	print_important "Do you want to package MediaElch ${ME_VERSION} for macOS with these settings?"
	print_important "It is recommended to clean your repository using \"git clean -fdx\" first."
	read -r -s -p "Press enter to continue, Ctrl+C to cancel"
	echo ""
}

if [[ "${1:-}" == "--help" ]] || [[ "${1:-}" == "help" ]]; then
	print_help
	exit 0
fi

# Update or download in case something is outdated
cd "${PROJECT_DIR}"
git submodule update --init
gather_information

no_confirm=${1:-confirm}
[ "${no_confirm}" != "--no-confirm" ] && confirm_build
echo ""

cd "${PROJECT_DIR}"

if [ ! -d "${BUILD_DIR}" ] || [ ! -f "${BUILD_DIR}/.qmake.stash" ] || [ ! -d "${BUILD_DIR}/MediaElch.app/Contents/MacOS" ]; then
	${SCRIPT_DIR}/../build_release.sh macOS
else
	print_info "Using existing build directory!"
fi

if [[ ! -f libmediainfo.0.dylib ]]; then
	wget --output-document MediaInfo_DLL.tar.bz2 https://mediaarea.net/download/binary/libmediainfo0/${MEDIAINFO_VERSION}/MediaInfo_DLL_${MEDIAINFO_VERSION}_Mac_i386+x86_64.tar.bz2
	tar -xvjf MediaInfo_DLL.tar.bz2
	mv MediaInfoLib/libmediainfo.0.dylib ./
	rm -rf MediaInfoLib
	rm MediaInfo_DLL.tar.bz2
fi

if [[ ! -d create-dmg ]]; then
	git clone https://github.com/andreyvit/create-dmg.git
	pushd "create-dmg" > /dev/null
	git checkout fbe0f36c823adbcbdcc15f9d65c6354252ac8307 > /dev/null # hard-coded version
	popd > /dev/null
fi

if [[ ! -f ffmpeg ]]; then
	wget --output-document ffmpeg.7z https://evermeet.cx/ffmpeg/ffmpeg-${FFMPEG_VERSION}.7z
	7za e ffmpeg.7z
	rm ffmpeg.7z
fi

pushd "${BUILD_DIR}" > /dev/null

cp ../ffmpeg MediaElch.app/Contents/MacOS/
cp ../libmediainfo.0.dylib MediaElch.app/Contents/MacOS/

print_info "Running macdeployqt"
macdeployqt MediaElch.app -qmldir=../src/ui -verbose=2

print_info "Running create-dmg"
../create-dmg/create-dmg \
	--volname "MediaElch" \
	--volicon "../MediaElch.icns" \
	--background "${SCRIPT_DIR}/macOS/backgroundImage.tiff" \
	--window-pos 200 120 \
	--window-size 550 400 \
	--icon-size 100 \
	--icon MediaElch.app 150 190 \
	--hide-extension MediaElch.app \
	--app-drop-link 400 190 \
	MediaElch_macOS_${ME_VERSION_NAME}.dmg \
	MediaElch.app

popd > /dev/null
