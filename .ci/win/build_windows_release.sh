#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1
PROJECT_DIR="$(pwd -P)"

source .ci/ci_utils.sh

# Specific to https://hub.docker.com/repository/docker/mediaelch/mediaelch-ci-win
export MXE_DIR="/build/mxe"
export MXE_TARGET="x86_64-w64-mingw32.shared"

export WIN_MEDIAINFO_URL="https://mediaarea.net/download/binary/libmediainfo0/${WIN_MEDIAINFO_VERSION}/MediaInfo_DLL_${WIN_MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z"

git submodule update --init -- third_party/quazip

mkdir -p third_party/packaging_win

if [[ ! -f third_party/packaging_win/MediaInfoDLL.7z ]]; then
	wget --output-document \
		third_party/packaging_win/MediaInfoDLL.7z \
		${WIN_MEDIAINFO_URL}
fi

if [[ ! -d MediaInfoDLL ]] || [[ ! -f third_party/packaging_win/MediaInfo.dll ]]; then
	# Delete in case on of them exists
	rm -rf MediaInfoDLL
	rm -rf third_party/packaging_win/MediaInfo.dll
	# Extract
	7zr x -othird_party/packaging_win/MediaInfo third_party/packaging_win/MediaInfoDLL.7z
	mv third_party/packaging_win/MediaInfo/Developers/Source/MediaInfoDLL ./MediaInfoDLL
	mv third_party/packaging_win/MediaInfo/MediaInfo.dll third_party/packaging_win/MediaInfo.dll
fi

if [[ ! -d ZenLib ]]; then
	svn checkout https://github.com/MediaArea/ZenLib/trunk/Source/ZenLib ./ZenLib
fi

cd "${PROJECT_DIR}"
mkdir -p build/win
cd build/win

qmake --version
qmake ../../MediaElch.pro \
	CONFIG+=release \
	LIBS+="${MXE_DIR}/usr/${MXE_TARGET}/bin/zlib1.dll"

make -j "${JOBS}"

echo ""
print_success "Successfuly built MediaElch! Release binary in"
print_success "  ${PROJECT_DIR}/build/win"
echo ""
