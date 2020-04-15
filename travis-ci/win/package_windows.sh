#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

cd "${0%/*}/../.."

# Specific to docker://archer96/mediaelch-mxe-qt
export MXE_DIR="/build/mxe"
export MXE_TARGET="x86_64-w64-mingw32.shared"
export MXE_LIB=${MXE_DIR}/usr/${MXE_TARGET}

export FFMPEG_VERSION="ffmpeg-latest-win64-static"

. ./scripts/utils.sh
. ./travis-ci/utils.sh

cd build/

# if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
# 	print_warning "Not packaging pull-requests for deployment"
# 	exit 0
# fi

# Check for required files.
if [ ! -f ../MediaInfo.dll ]; then
	print_error "MediaInfo.dll not found! Should have been downloaded in build_windows_mxe.sh"
	exit 1
fi

print_info "Assembling package - Copying DLLs"
rm -rf pkg-zip
mkdir -p pkg-zip/MediaElch
cp release/MediaElch.exe pkg-zip/MediaElch/


while IFS= read -r file; do
	cp ${MXE_LIB}/${file} pkg-zip/MediaElch/
done < "../travis-ci/win/dll_list.txt"

mkdir -p pkg-zip/MediaElch/sqldrivers
cp ${MXE_LIB}/qt5/plugins/sqldrivers/qsqlite.dll  pkg-zip/MediaElch/sqldrivers

mkdir -p pkg-zip/MediaElch/platforms
cp ${MXE_LIB}/qt5/plugins/platforms/qwindows.dll  pkg-zip/MediaElch/platforms
cp ${MXE_LIB}/qt5/plugins/platforms/qminimal.dll  pkg-zip/MediaElch/platforms

mkdir -p pkg-zip/MediaElch/styles
cp ${MXE_LIB}/qt5/plugins/styles/qwindowsvistastyle.dll  pkg-zip/MediaElch/styles

mkdir -p pkg-zip/MediaElch/QtQuick/Controls
cp ${MXE_LIB}/qt5/qml/QtQuick/Controls/qmldir                    pkg-zip/MediaElch/QtQuick/Controls
cp ${MXE_LIB}/qt5/qml/QtQuick/Controls/qtquickcontrolsplugin.dll pkg-zip/MediaElch/QtQuick/Controls

cp -R ${MXE_LIB}/qt5/qml/QtQml/            pkg-zip/MediaElch/
cp -R ${MXE_LIB}/qt5/qml/QtQuick.2/        pkg-zip/MediaElch/
cp -R ${MXE_LIB}/qt5/plugins/imageformats/ pkg-zip/MediaElch/
cp -R ${MXE_LIB}/qt5/plugins/mediaservice/ pkg-zip/MediaElch/

print_info "Copying MediaInfo.dll"
cp ../MediaInfo.dll pkg-zip/MediaElch/

if [[ ! -f ${FFMPEG_VERSION}/bin/ffmpeg.exe ]]; then
	print_info "Downloading and copying ffmpeg.exe"
	wget --output-document ffmpeg.zip https://ffmpeg.zeranoe.com/builds/win64/static/${FFMPEG_VERSION}.zip
	unzip ffmpeg.zip ${FFMPEG_VERSION}/bin/ffmpeg.exe
fi
mkdir pkg-zip/MediaElch/vendor
cp ${FFMPEG_VERSION}/bin/ffmpeg.exe pkg-zip/MediaElch/vendor/

print_info "Zipping 'MediaElch_win.zip'"
cd pkg-zip
rm -f ../MediaElch_win.zip
zip -r "../MediaElch_win.zip" ./*
