#!/usr/bin/env bash

###########################################################
#
# Travis CI - Package for bintray.com
#
# This script packages MediaElch for
#  - linux (AppImage, see https://appimage.org/)
#  - macOS (dmg)
#  - windows (zipped exe with dlls)
#
# It is used by TravisCI to deploy MediaElch nightlies
# to bintray.com.
#
###########################################################

# Exit on errors
set -e

if [ -z ${QT+x} ]; then print_error "\$QT is unset"; return 1; fi

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
. "${SCRIPT_DIR}/utils.sh"
. "${SCRIPT_DIR}/../scripts/utils.sh"

if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
	print_warning "Not packaging pull-requests for deployment"
	exit 0
fi

if [ -f "${SCRIPT_DIR}/defs.sh" ]; then
	. "${SCRIPT_DIR}/defs.sh" # Contains MXETARGET, etc.
fi

# Gather information for packaging MediaElch such as version, git
# hash and date.
gather_information() {
	pushd "${PROJECT_DIR}" > /dev/null

	GIT_DATE=$(git --git-dir=".git" show --no-patch --pretty="%ci")
	echo "GIT_DATE = ${GIT_DATE}"

	if [ ${OS_NAME} = "Linux" ]; then
		if [ $QT = "qtWin" ]; then
			TARGET_OS="win"
			FILE_TYPE="zip"
		else
			TARGET_OS="linux"
			FILE_TYPE="AppImage"
		fi
		RELEASE_DATE=$(date -u +"%Y-%m-%dT%H:%M:%S%z" --date="${GIT_DATE}")

	elif [ ${OS_NAME} = "Darwin" ]; then
		TARGET_OS="macOS"
		FILE_TYPE="dmg"
		RELEASE_DATE=$(date -ujf "%Y-%m-%d %H:%M:%S %z" "${GIT_DATE}" "+%Y-%m-%dT%H:%M:%S%z")

	else
		print_error "Unknown operating system: ${OS_NAME}"
		exit 1
	fi
	echo "RELEASE_DATE = ${RELEASE_DATE}"

	ME_VERSION=$(sed -ne 's/.*AppVersionFullStr[^"]*"\(.*\)";.*/\1/p' Version.h)
	echo "ME_VERSION = ${ME_VERSION}"

	GIT_HASH=$(git --git-dir=".git" show --no-patch --pretty="%h")
	echo "GIT_HASH = ${GIT_HASH}"

	DATE_HASH=$(date -u +"%Y-%m-%d_%H-%M")
	echo "DATE_HASH = ${DATE_HASH}"

	DATE_DESC=$(date -u +"%Y-%m-%d %H:%M")
	echo "DATE_DESC = ${DATE_DESC}"

	VERSION_NAME="${ME_VERSION}_${DATE_HASH}_git-${TRAVIS_BRANCH}-${GIT_HASH}"
	echo "VERSION_NAME = ${VERSION_NAME}"

	popd > /dev/null
}

# Create an AppImage file.
# For more about AppImage files, see: https://appimage.org/
create_appimage() {
	print_important "Create an AppImage release"

	# Workaround for: https://github.com/probonopd/linuxdeployqt/issues/65
	unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH
	export PATH="/opt/${QT}/bin:$PATH"
	# linuxdeployqt uses this for naming the file
	export VERSION=$ME_VERSION

	#######################################################
	# Download linuxdeployqt

	fold_start "download_linuxdeployqt"
	print_info "Downloading linuxdeployqt"
	if [ ! -f linuxdeploy-x86_64.AppImage ]; then
		wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
		chmod u+x linuxdeploy*.AppImage
	fi
	if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
		wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
		chmod u+x linuxdeploy*.AppImage
	fi
	fold_end

	#######################################################
	# Install MediaElch into subdirectory

	fold_start "install_mediaelch"
	print_info "Installing MediaElch in subdirectory to create basic AppDir structure"
	make INSTALL_ROOT=appdir -j${JOBS} install
	find appdir/
	fold_end

	#######################################################
	# Copy libmediainfo
	#
	# libmediainfo.so.0 is loaded at runtime that's why
	# linuxdeployqt can't detect it and we have to include
	# it here.

	fold_start "copy_libmediainfo"
	print_info "Copying libmediainfo.so"
	mkdir -p ./appdir/usr/lib
	cp /usr/lib/x86_64-linux-gnu/libmediainfo.so.0 ./appdir/usr/lib/
	fold_end

	#######################################################
	# Download and copy ffmpeg

	fold_start "ffmpeg"
	print_info "Downloading ffmpeg"
	# Use static ffmpeg
	wget -c https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz -O ffmpeg.tar.xz
	ffmpeg_md5="bb86a3fda0fc567af6ef208a0d507d32  ffmpeg.tar.xz"
	if [ "$(md5sum ffmpeg.tar.xz)" = "${ffmpeg_md5}" ]; then
		print_info "FFMPEG MD5 checksum is valid"
	else
		print_error "MD5 checksum no valid"
		print_error "  Expected: ${ffmpeg_md5}"
		print_error "  Was:      $(md5sum ffmpeg.tar.xz)"
		exit 1
	fi
	tar -xJvf ffmpeg.tar.xz
	print_info "Copying ffmpeg into AppDir"
	cp ffmpeg-*/ffmpeg appdir/usr/bin/
	fold_end

	#######################################################
	# Create AppImage

	fold_start "linuxdeployqt"
	print_info "Running linuxdeployqt"
	print_important "Creating an AppImage for MediaElch ${VERSION_NAME}"
	export QML_SOURCES_PATHS="${PROJECT_DIR}/src/ui"
	# Run linuxdeploy with following settings:
	# - use qt plugin       => so that Qt libraries are bundled correctly
	# - use appimage plugin => create an appimage file (essentially just bundles the appdir)
	"./linuxdeploy-x86_64.AppImage" --appdir appdir                    \
		--desktop-file appdir/usr/share/applications/MediaElch.desktop \
		--plugin qt                                                    \
		--output appimage
	find . -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq
	fold_end

	#######################################################
	# Finalize AppImage (name, chmod)

	fold_start "renaming"
	print_info "Renaming .AppImage"
	mv MediaElch-${VERSION}*.AppImage MediaElch_linux_${VERSION_NAME}.AppImage
	chmod +x ./*.AppImage
	fold_end
}

create_macos_dmg() {
	print_important "Create macOS .dmg file"

	# Check for required files.
	if [ ! -f ../libmediainfo.0.dylib ]; then
		print_error "libmediainfo.0.dylib not found! Should have been downloaded in install_dependencies.sh"
		exit 1
	fi

	#######################################################
	# Install create-dmg
	git clone https://github.com/andreyvit/create-dmg.git

	#######################################################
	# Installing 7zip

	print_info "Installing p7zip using brew"
	brew install p7zip

	#######################################################
	# ffmpeg

	fold_start "ffmpeg"
	print_info "Downloading and copying ffmpeg into MediaElch.dmg"
	wget --output-document ffmpeg.7z https://evermeet.cx/ffmpeg/ffmpeg-4.0.7z
	7za e ffmpeg.7z
	cp ffmpeg MediaElch.app/Contents/MacOS/
	fold_end

	#######################################################
	# MediaInfoDLL

	fold_start "mediainfo"
	print_info "Copying mediainfo into MediaElch.dmg"
	cp ../libmediainfo.0.dylib MediaElch.app/Contents/MacOS/
	fold_end

	#######################################################
	# Creating a "beautiful" .dmg

	fold_start "dmg"
	print_important "Running macdeployqt"
	/usr/local/opt/qt/bin/macdeployqt MediaElch.app -qmldir=../src/ui -verbose=2
	print_info "Running create-dmg"
	create-dmg/create-dmg \
		--volname "MediaElch" \
		--volicon "../MediaElch.icns" \
		--background "${SCRIPT_DIR}/macOS/backgroundImage.tiff" \
		--window-pos 200 120 \
		--window-size 550 400 \
		--icon-size 100 \
		--icon MediaElch.app 150 190 \
		--hide-extension MediaElch.app \
		--app-drop-link 400 190 \
		MediaElch_macOS_${VERSION_NAME}.dmg \
		MediaElch.app
	fold_end
}

package_zip() {
	print_info "package build into zip for win"

	# Check for required files.
	if [ ! -f ../MediaInfo.dll ]; then
		print_error "MediaInfo.dll not found! Should have been downloaded in install_dependencies.sh"
		exit 1
	fi

	fold_start "unzip"
	print_info "Installing unzip"
	sudo apt-get install unzip
	fold_end

	fold_start "copy_dlls"
	print_info "Assembling package - Copying DLLs"
	mkdir -p pkg-zip/MediaElch
	cp release/MediaElch.exe pkg-zip/MediaElch/

	local MXELIB=${MXEDIR}/usr/${MXETARGET}
	local FFMPEG_VERSION="ffmpeg-latest-win64-static"

	while IFS= read -r file; do
		cp ${MXELIB}/${file} pkg-zip/MediaElch/
	done < "${SCRIPT_DIR}/win/dll_list.txt"

	mkdir -p pkg-zip/MediaElch/sqldrivers
	cp ${MXELIB}/qt5/plugins/sqldrivers/qsqlite.dll                 pkg-zip/MediaElch/sqldrivers

	mkdir -p pkg-zip/MediaElch/platforms
	cp ${MXELIB}/qt5/plugins/platforms/qwindows.dll                 pkg-zip/MediaElch/platforms
	cp ${MXELIB}/qt5/plugins/platforms/qminimal.dll                 pkg-zip/MediaElch/platforms

	mkdir -p pkg-zip/MediaElch/styles
	cp ${MXELIB}/qt5/plugins/styles/qwindowsvistastyle.dll          pkg-zip/MediaElch/styles

	mkdir -p pkg-zip/MediaElch/QtQuick/Controls
	cp ${MXELIB}/qt5/qml/QtQuick/Controls/qmldir                    pkg-zip/MediaElch/QtQuick/Controls
	cp ${MXELIB}/qt5/qml/QtQuick/Controls/qtquickcontrolsplugin.dll pkg-zip/MediaElch/QtQuick/Controls

	cp -R ${MXELIB}/qt5/qml/QtQml/            pkg-zip/MediaElch/
	cp -R ${MXELIB}/qt5/qml/QtQuick.2/        pkg-zip/MediaElch/
	cp -R ${MXELIB}/qt5/plugins/imageformats/ pkg-zip/MediaElch/
	cp -R ${MXELIB}/qt5/plugins/mediaservice/ pkg-zip/MediaElch/
	fold_end

	print_info "Copying MediaInfo.dll"
	cp ../MediaInfo.dll pkg-zip/MediaElch/

	fold_start "ffmpeg_exe"
	print_info "Downloading and copying ffmpeg.exe"
	wget --output-document ffmpeg.zip https://ffmpeg.zeranoe.com/builds/win64/static/${FFMPEG_VERSION}.zip
	unzip ffmpeg.zip ${FFMPEG_VERSION}/bin/ffmpeg.exe
	mkdir pkg-zip/MediaElch/vendor
	cp ${FFMPEG_VERSION}/bin/ffmpeg.exe pkg-zip/MediaElch/vendor/
	fold_end

	fold_start "zipping_exe"
	print_info "Zipping 'MediaElch_win_${VERSION_NAME}.zip'"
	pushd pkg-zip > /dev/null
	zip -r "../MediaElch_win_${VERSION_NAME}.zip" ./*
	popd > /dev/null
	fold_end
}

# Creates the bintray json.
create_bintray_json() {
	print_info "Preparing bintray.json for ${TARGET_OS}"

	cat > "${SCRIPT_DIR}/bintray.json" <<EOF
{
	"package": {
		"name": "MediaElch-${TARGET_OS}",
		"repo": "MediaElch",
		"subject": "komet",
		"website_url": "https://www.kvibes.de/mediaelch/",
		"vcs_url": "https://github.com/Komet/MediaElch.git",
		"issue_tracker_url": "https://github.com/Komet/MediaElch/issues",
		"licenses": ["LGPL-3.0"]
	},
	"version": {
		"name": "${VERSION_NAME}",
		"desc": "MediaElch version ${ME_VERSION} for ${TARGET_OS}\nDate: ${DATE_DESC}\nGit Branch: ${TRAVIS_BRANCH}\nGit Hash: ${GIT_HASH}",
		"released": "${RELEASE_DATE}",
		"gpgSign": false
	},
	"files":
	[
		{
			"includePattern": "${PROJECT_DIR}/build/MediaElch_${TARGET_OS}_${VERSION_NAME}.${FILE_TYPE}",
			"uploadPattern": "MediaElch_${TARGET_OS}_${VERSION_NAME}.${FILE_TYPE}"
		}
	],
	"publish": true
}
EOF
}

print_important "Packaging ${QT} (${OS_NAME}) for deployment"
pushd "${PROJECT_DIR}/build" > /dev/null

gather_information

if [ "${TARGET_OS}" = "linux" ]; then
	if [ $QT = "qt55" ]; then
		print_info "Not deploying AppImage for old Qt version."
		exit 0
	else
		create_appimage
	fi

elif [ "${TARGET_OS}" = "win" ]; then
	package_zip

elif [ "${TARGET_OS}" = "macOS" ]; then
	create_macos_dmg

else
	print_error "Unknown operating system: ${OS_NAME}"
	exit 1
fi

create_bintray_json

echo "cat \"${SCRIPT_DIR}/bintray.json\""
cat "${SCRIPT_DIR}/bintray.json"

print_info "Deployment preparation successful"
popd > /dev/null
