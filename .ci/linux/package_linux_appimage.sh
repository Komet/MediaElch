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

print_help() {
	cat << EOF
Usage: $(basename "$0") [options]

This script builds and packages MediaElch in a distributable form for
Linux distributions.  It is used by MediaElch authors to release
official MediaElch releases.

    DO NOT USE THIS IF YOU'RE AN END-USER
    UNLESS YOU KNOW WHAT YOU'RE DOING!

Use .ci/linux/build_linux_release.sh if you want to build MediaElch.

See https://appimage.org/

Options
  --no-confirm   Package MediaElch without confirm dialog.
EOF
	exit
}

confirm_build() {
	echo ""
	print_important "Do you want to package MediaElch ${ME_VERSION} for Linux with these settings?"
	print_important "It is recommended to clean your repository using \"git clean -fdx\"."
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

# Check for linux build and packaging dependencies
source .ci/linux/check_linux_dependencies.sh
check_dependencies_linux
check_dependencies_linux_appimage

# Exports required variables
gather_project_and_system_details

if [[ "${NO_CONFIRM}" != "1" ]]; then
	confirm_build
fi
echo ""

#######################################################
# Set environment variables

# Workaround for: https://github.com/probonopd/linuxdeployqt/issues/65
unset QTDIR
unset QT_PLUGIN_PATH
unset LD_LIBRARY_PATH
# linuxdeployqt uses $VERSION this for naming the file
VERSION="${ME_VERSION}"
QML_SOURCES_PATHS="${PROJECT_DIR}/src/ui"
EXTRA_QT_PLUGINS="qt5dxcb-plugin"

export VERSION
export QML_SOURCES_PATHS
export EXTRA_QT_PLUGINS

if [[ ! "$PATH" = *"qt"* ]] && [[ ! "$PATH" = *"Qt"* ]]; then
	print_fatal "/path/to/qt/bin must be in your \$PATH, e.g. \nexport PATH=\"\$PATH:/usr/lib/x86_64-linux-gnu/qt5/bin\""
fi

if [[ "$(qmlimportscanner)" = *"could not find a Qt installation"* ]]; then
	print_fatal "qmlimportscanner could not find a Qt installation.\nInstall qtdeclarative5-dev-tools\"."
fi

#######################################################
# Check that MediaElch is build

if [ ! -d "./build/linux" ] || [ ! -f "./build/linux/.qmake.stash" ] || [ ! -f "./build/linux/MediaElch" ]; then
	print_fatal "Build MediaElch before packaging it!"
fi

#######################################################
# Download Dependencies into third_party folder

mkdir -p "${PROJECT_DIR}/third_party/packaging_linux"
cd "${PROJECT_DIR}/third_party/packaging_linux"

#######################################################
# Download linuxdeployqt + Qt plugin

if [[ ! -f linuxdeploy-x86_64.AppImage ]]; then
	print_info "Downloading linuxdeploy"
	wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
	chmod u+x linuxdeploy*.AppImage
fi

if [[ ! -d linuxdeploy-extracted ]]; then
	print_info "Extracting linuxdeploy"
	./linuxdeploy-x86_64.AppImage --appimage-extract
	mv squashfs-root linuxdeploy-extracted
fi

if [[ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]]; then
	print_info "Downloading linuxdeployqt"
	wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
	chmod u+x linuxdeploy*.AppImage
fi

if [[ ! -d linuxdeploy-extracted/plugins/linuxdeploy-plugin-qt ]]; then
	print_info "Extracting linuxdeployqt"
	./linuxdeploy-plugin-qt-x86_64.AppImage --appimage-extract
	mv squashfs-root linuxdeploy-extracted/plugins/linuxdeploy-plugin-qt
	cd linuxdeploy-extracted/usr/bin
	ln -s ../../plugins/linuxdeploy-plugin-qt/AppRun linuxdeploy-plugin-qt
	cd ../../..
fi

#######################################################
# Download and extract ffmpeg

if [[ ! -f ffmpeg.tar.xz ]]; then
	print_info "Downloading ffmpeg"
	# Use static ffmpeg
	wget -c https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz -O ffmpeg.tar.xz
	ACTUAL_SHA512="$(shasum -a 512 ffmpeg.tar.xz)"
	if [ "${ACTUAL_SHA512}" = "${LINUX_FFMPEG_SHA512}" ]; then
		print_info "FFMPEG SHA512 checksum is valid"
	else
		print_error "SHA512 checksum no valid"
		print_error "  Expected: ${LINUX_FFMPEG_SHA512}"
		print_error "  Was:      ${ACTUAL_SHA512}"
		exit 1
	fi
fi

if [[ ! -f ffmpeg ]]; then
	tar -xJvf ffmpeg.tar.xz
	mv ffmpeg-*-static/ffmpeg ./
	rm -rf ffmpeg-*-static
fi

#######################################################
# Install MediaElch into subdirectory

cd "${PROJECT_DIR}/build/linux"
print_important "Create an AppImage release"
print_info "Installing MediaElch into subdirectory to create basic AppDir structure"
rm -rf appdir
make INSTALL_ROOT=appdir -j"${JOBS}" install

print_info "Listing appdir"
tree appdir/

#######################################################
# Copy libmediainfo
#
# libmediainfo.so.0 is loaded at runtime that's why
# linuxdeployqt can't detect it and we have to include
# it here.

print_info "Copying libmediainfo.so"
mkdir -p ./appdir/usr/lib
cp /usr/lib/x86_64-linux-gnu/libmediainfo.so.0 ./appdir/usr/lib/

#######################################################
# Copy ffmpeg

print_info "Copying ffmpeg into AppDir"
cp "${PROJECT_DIR}/third_party/packaging_linux/ffmpeg" appdir/usr/bin/

#######################################################
# Create AppImage

print_important "Creating an AppImage for MediaElch ${ME_VERSION_NAME}."
print_important "This takes a while and may seem frozen."

# Run linuxdeploy with following settings:
# - use qt plugin       => so that Qt libraries are bundled correctly
# - use appimage plugin => create an AppImage file (essentially just bundles the appdir)
"${PROJECT_DIR}/third_party/packaging_linux/linuxdeploy-extracted/AppRun" \
	--appdir appdir \
	--desktop-file appdir/usr/share/applications/MediaElch.desktop \
	--plugin qt \
	--output appimage

#######################################################
# Finalize AppImage (name, chmod)

cd "${PROJECT_DIR}/build/linux"
print_info "Renaming .AppImage"
chmod +x ./*.AppImage
mv MediaElch*.AppImage "../../MediaElch_linux_${ME_VERSION_NAME}.AppImage"
cd ../..

print_success "Successfully created AppImage: "
print_success "    $(pwd)/MediaElch_linux_${ME_VERSION_NAME}.AppImage"
