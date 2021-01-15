#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1

#######################################################
# Variables & Functions

PROJECT_DIR="$(pwd -P)"
FILE_TYPE="AppImage"

source .ci/ci_utils.sh

# Specific to docker://archer96/mediaelch-mxe-qt
export MXE_DIR="/build/mxe"
export MXE_TARGET="x86_64-w64-mingw32.shared"
export MXE_LIB=${MXE_DIR}/usr/${MXE_TARGET}

#######################################################
# Getting Details

# Exports required variables
gather_project_and_system_details

#######################################################
# Download Dependencies into third_party folder

mkdir -p "${PROJECT_DIR}/third_party/packaging_win"
cd "${PROJECT_DIR}/third_party/packaging_win"

#######################################################
# Download and extract ffmpeg

if [[ ! -f ${WIN_FFMPEG_VERSION}/bin/ffmpeg.exe ]]; then
	print_info "Downloading and copying ffmpeg.exe"
	wget --output-document ffmpeg.zip https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip

	if [ "$(shasum -a 512 ffmpeg.zip)" = "${WIN_FFMPEG_SHA512}" ]; then
		print_info "FFMPEG SHA512 checksum is valid"
	else
		print_error "SHA512 checksum no valid"
		print_error "  Expected: ${WIN_FFMPEG_SHA512}"
		print_error "  Was:      $(shasum -a 512 ffmpeg.zip)"
		exit 1
	fi
	unzip -o ffmpeg.zip ${WIN_FFMPEG_VERSION}/bin/ffmpeg.exe
fi

#######################################################
# Copy Dependencies

cd "${PROJECT_DIR}/build/win"

print_info "Assembling package - Copying DLLs"
rm -rf pkg-zip
mkdir -p pkg-zip/MediaElch
cp release/MediaElch.exe pkg-zip/MediaElch/

while IFS= read -r file; do
	cp ${MXE_LIB}/${file} pkg-zip/MediaElch/
done < "${PROJECT_DIR}/.ci/win/dll_list.txt"

mkdir -p pkg-zip/MediaElch/sqldrivers
cp ${MXE_LIB}/qt5/plugins/sqldrivers/qsqlite.dll pkg-zip/MediaElch/sqldrivers

mkdir -p pkg-zip/MediaElch/platforms
cp ${MXE_LIB}/qt5/plugins/platforms/qwindows.dll pkg-zip/MediaElch/platforms
cp ${MXE_LIB}/qt5/plugins/platforms/qminimal.dll pkg-zip/MediaElch/platforms

mkdir -p pkg-zip/MediaElch/styles
cp ${MXE_LIB}/qt5/plugins/styles/qwindowsvistastyle.dll pkg-zip/MediaElch/styles

mkdir -p pkg-zip/MediaElch/QtQuick/Controls
cp ${MXE_LIB}/qt5/qml/QtQuick/Controls/qmldir pkg-zip/MediaElch/QtQuick/Controls
cp ${MXE_LIB}/qt5/qml/QtQuick/Controls/qtquickcontrolsplugin.dll pkg-zip/MediaElch/QtQuick/Controls

cp -R ${MXE_LIB}/qt5/qml/QtQml/ pkg-zip/MediaElch/
cp -R ${MXE_LIB}/qt5/qml/QtQuick.2/ pkg-zip/MediaElch/
cp -R ${MXE_LIB}/qt5/plugins/imageformats/ pkg-zip/MediaElch/
cp -R ${MXE_LIB}/qt5/plugins/mediaservice/ pkg-zip/MediaElch/

print_info "Copying MediaInfo.dll"
cp "${PROJECT_DIR}/third_party/packaging_win/MediaInfo.dll" pkg-zip/MediaElch/

mkdir pkg-zip/MediaElch/vendor
cp "${PROJECT_DIR}/third_party/packaging_win/${WIN_FFMPEG_VERSION}/bin/ffmpeg.exe" pkg-zip/MediaElch/vendor/

#######################################################
# Finalize Zip (name, chmod)

print_info "Zipping 'MediaElch_win.zip'"
cd pkg-zip
rm -f ../MediaElch_win.zip
zip -r "../MediaElch_win.zip" ./*
cd ..
mv MediaElch_win.zip "${PROJECT_DIR}/MediaElch_win_${ME_VERSION_NAME}.zip"

print_success "Successfully created Windows ZIP: "
print_success "    ${PROJECT_DIR}/MediaElch_win_${ME_VERSION_NAME}.zip"
