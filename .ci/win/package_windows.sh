#!/usr/bin/env bash

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1

#######################################################
# Variables & Functions

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

if [[ ! -f ${WIN_FFMPEG_FOLDER_NAME}/bin/ffmpeg.exe ]]; then
	print_info "Downloading and copying ffmpeg.exe from ${WIN_FFMPEG_URL:?}"
	wget --no-verbose --output-document "${WIN_FFMPEG_ARCHIVE_NAME:?}" "${WIN_FFMPEG_URL:?}"

	validate_sha512 "${WIN_FFMPEG_ARCHIVE_NAME:?}" "${WIN_FFMPEG_SHA512:?}"

	unzip -o "${WIN_FFMPEG_ARCHIVE_NAME:?}" "${WIN_FFMPEG_FOLDER_NAME:?}/bin/ffmpeg.exe"
fi

if [[ ! -f opengl32sw.dll ]]; then
	wget --no-verbose --output-document opengl32sw.dll https://github.com/mediaelch/mediaelch-dep/blob/main/opengl32sw.dll?raw=true
fi

#######################################################
# Copy Dependencies

cd "${PROJECT_DIR}/build/win-qt${QT_MAJOR_VERSION:?}"

print_info "Assembling package - Copying DLLs"
rm -rf pkg-zip
mkdir -p pkg-zip/MediaElch
cp release/MediaElch.exe pkg-zip/MediaElch/

while IFS= read -r file; do
	if [[ ${file:0:1} != \# ]] ; then
		cp "${MXE_LIB}/${file:?}" pkg-zip/MediaElch/
	fi
done < "${PROJECT_DIR}/.ci/win/dll_list_qt${QT_MAJOR_VERSION:?}.txt"

mkdir -p pkg-zip/MediaElch/sqldrivers
cp "${MXE_LIB}/qt${QT_MAJOR_VERSION}/plugins/sqldrivers/qsqlite.dll" pkg-zip/MediaElch/sqldrivers

mkdir -p pkg-zip/MediaElch/platforms
cp "${MXE_LIB}/qt${QT_MAJOR_VERSION}/plugins/platforms/qwindows.dll" pkg-zip/MediaElch/platforms
cp "${MXE_LIB}/qt${QT_MAJOR_VERSION}/plugins/platforms/qminimal.dll" pkg-zip/MediaElch/platforms


cp -R "${MXE_LIB}/qt${QT_MAJOR_VERSION}/plugins/imageformats/" pkg-zip/MediaElch/

if [[ "${QT_MAJOR_VERSION}" = "5" ]]; then
	mkdir -p pkg-zip/MediaElch/styles
	cp "${MXE_LIB}/qt${QT_MAJOR_VERSION}/plugins/styles/qwindowsvistastyle.dll" pkg-zip/MediaElch/styles

	cp -R "${MXE_LIB}/qt${QT_MAJOR_VERSION}/plugins/mediaservice/" pkg-zip/MediaElch/

	print_info "Copying opengl32sw.dll"
	cp "${PROJECT_DIR}/third_party/packaging_win/opengl32sw.dll" pkg-zip/MediaElch/

elif [[ "${QT_MAJOR_VERSION}" = "6" ]]; then
	mkdir -p pkg-zip/MediaElch/styles
	cp "${MXE_LIB}/qt${QT_MAJOR_VERSION}/plugins/styles/qmodernwindowsstyle.dll" pkg-zip/MediaElch/styles

	mkdir -p pkg-zip/MediaElch/iconengines
	cp "${MXE_LIB}/qt${QT_MAJOR_VERSION}/plugins/iconengines/qsvgicon.dll" pkg-zip/MediaElch/iconengines/

	cp -R "${MXE_LIB}/qt${QT_MAJOR_VERSION}/plugins/tls/" pkg-zip/MediaElch/
fi

print_info "Copying MediaInfo.dll"
cp "${PROJECT_DIR}/third_party/packaging_win/MediaInfo.dll" pkg-zip/MediaElch/

mkdir pkg-zip/MediaElch/vendor
cp "${PROJECT_DIR}/third_party/packaging_win/${WIN_FFMPEG_FOLDER_NAME}/bin/ffmpeg.exe" pkg-zip/MediaElch/vendor/

#########################################
# Translations

QT_TRANSLATIONS_PATH="$(qmake -query QT_INSTALL_TRANSLATIONS)"
print_info "Copying Qt's translations from ${QT_TRANSLATIONS_PATH}"
mkdir -p pkg-zip/MediaElch/translations

if [[ "${QT_MAJOR_VERSION}" = "6" ]]; then
	# Note: Even qtscript_XX.qm is required. *.qm files seem to depend on each other.
	cp "${QT_TRANSLATIONS_PATH}"/qt*.qm ./pkg-zip/MediaElch/translations/

	# Check that translations were copied by looking for German
	if [[ ! -f ./pkg-zip/MediaElch/translations/qt_de.qm ]]; then
		print_fatal "German translation for Qt is missing!"
	fi
fi

#######################################################
# Finalize Zip (name, chmod)

print_info "Zipping 'MediaElch_win.zip'"
cd pkg-zip
rm -f ../MediaElch_win.zip
zip -r "../MediaElch_win.zip" ./*
cd ..
mv MediaElch_win.zip "${PROJECT_DIR}/MediaElch_win_Qt${QT_MAJOR_VERSION}_${ME_VERSION_NAME}.zip"

print_success "Successfully created Windows ZIP: "
print_success "    ${PROJECT_DIR}/MediaElch_win_Qt${QT_MAJOR_VERSION}_${ME_VERSION_NAME}.zip"
