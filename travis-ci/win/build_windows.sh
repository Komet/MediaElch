#!/usr/bin/env bash

set -euo pipefail
IFS=$'\n\t'

cd "${0%/*}/../.."

# Specific to docker://archer96/mediaelch-mxe-qt
export MXE_DIR="/build/mxe"
export MXE_TARGET="x86_64-w64-mingw32.shared"

export MEDIAINFO_VERSION="20.03"
export MEDIAINFO_URL="https://mediaarea.net/download/binary/libmediainfo0/${MEDIAINFO_VERSION}/MediaInfo_DLL_${MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z"


git submodule update --init -- third_party/quazip

if [[ ! -d MediaInfo ]]; then
	wget --output-document MediaInfoDLL.7z ${MEDIAINFO_URL}
	7zr x -oMediaInfo MediaInfoDLL.7z
	mv MediaInfo/Developers/Source/MediaInfoDLL ./MediaInfoDLL
	mv MediaInfo/MediaInfo.dll MediaInfo.dll
	rm -rf MediaInfo MediaInfoDLL.7z
fi

if [[ ! -d ZenLib ]]; then
	svn checkout https://github.com/MediaArea/ZenLib/trunk/Source/ZenLib ./ZenLib
fi


mkdir -p build
cd build

qmake --version
qmake ../MediaElch.pro \
	CONFIG+=release \
	LIBS+="${MXE_DIR}/usr/${MXE_TARGET}/bin/zlib1.dll"


make -j "$(nproc)"
