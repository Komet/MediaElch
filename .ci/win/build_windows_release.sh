#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1
PROJECT_DIR="$(pwd -P)"

source .ci/ci_utils.sh

if [[ ! -v QT_MAJOR_VERSION ]]; then
	print_fatal "Expected \$QT_MAJOR_VERSION to be set to either 5 or 6; is unset"
elif [[ "${QT_MAJOR_VERSION:-}" != "5" ]] && [[ "${QT_MAJOR_VERSION:-}" != "6" ]]; then
	print_fatal "Expected \$QT_MAJOR_VERSION to be set to either 5 or 6; is: ${QT_MAJOR_VERSION:-}"
fi

# Specific to https://hub.docker.com/repository/docker/mediaelch/mediaelch-ci-win
export MXE_DIR="/build/mxe"
export MXE_TARGET="x86_64-w64-mingw32.shared"

git submodule update --init -- third_party/quazip

mkdir -p third_party/packaging_win

if [[ ! -f third_party/packaging_win/MediaInfoDLL.7z ]]; then
	wget --no-verbose --output-document \
		third_party/packaging_win/MediaInfoDLL.7z \
		"${WIN_MEDIAINFO_URL}"
	validate_sha512 "third_party/packaging_win/MediaInfoDLL.7z" "${WIN_MEDIAINFO_SHA512}"
fi

if [[ ! -d MediaInfoDLL ]] || [[ ! -f third_party/packaging_win/MediaInfo.dll ]]; then
	# Delete in case on of them exists
	rm -rf MediaInfoDLL
	rm -rf third_party/packaging_win/MediaInfo.dll
	# Extract
	7zr x -bd -bb0 -aoa -othird_party/packaging_win/MediaInfo third_party/packaging_win/MediaInfoDLL.7z
	mv third_party/packaging_win/MediaInfo/Developers/Source/MediaInfoDLL ./MediaInfoDLL
	mv third_party/packaging_win/MediaInfo/MediaInfo.dll third_party/packaging_win/MediaInfo.dll
fi

if [[ ! -d "ZenLib" ]]; then
	print_info "Loading ZenLib"
	mkdir -p tmp
	rm -rf tmp/ZenLib # clean up in case previous runs were aborted
	git clone --quiet --depth=1 --single-branch --branch=master https://github.com/MediaArea/ZenLib/ tmp/ZenLib
	mv tmp/ZenLib/Source/ZenLib ./ZenLib
	rm -rf tmp
fi

cd "${PROJECT_DIR}"
mkdir -p "build/win-qt${QT_MAJOR_VERSION}"
cd "build/win-qt${QT_MAJOR_VERSION}"

qmake --version
qmake ../../MediaElch.pro \
	CONFIG+=release \
	LIBS+="${MXE_DIR}/usr/${MXE_TARGET}/bin/zlib1.dll"

make -j "${JOBS}"

echo ""
print_success "Successfuly built MediaElch! Release binary in"
print_success "  ${PROJECT_DIR}/build/win-qt${QT_MAJOR_VERSION}"
echo ""
