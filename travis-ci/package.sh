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

	ME_VERSION=$(sed -ne 's/.*setApplicationVersion("\(.*\)");/\1/p' main.cpp)
	echo "ME_VERSION = ${ME_VERSION}"

	GIT_HASH=$(git --git-dir=".git" show --no-patch --pretty="%h")
	echo "GIT_HASH = ${GIT_HASH}"

	DATE_HASH=$(date -u +"%Y-%m-%d_%H-%M")
	echo "DATE_HASH = ${DATE_HASH}"

	VERSION_NAME="${ME_VERSION}_${DATE_HASH}_git-${TRAVIS_BRANCH}-${GIT_HASH}"
	echo "VERSION_NAME = ${VERSION_NAME}"
}

# Create an AppImage file.
# For more about AppImage files, see: https://appimage.org/
create_appimage() {
	print_important "Create an AppImage release"
	pushd ${PROJECT_DIR} > /dev/null

	fold_start "download_linuxdeployqt"
	print_info "Downloading linuxdeployqt"
	wget --output-document linuxdeployqt.AppImage https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
	chmod a+x ./linuxdeployqt.AppImage
	fold_end

	unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH
	export PATH="/opt/${QT}/bin:$PATH"
	export VERSION=$ME_VERSION # linuxdeployqt uses this for naming the file

	fold_start "cleanup"
	print_info "Cleanup temporary build files"
	# Recommended by linuxdeployqt
	find . \( -name "moc_*" -or -name "ui_*" -or -name "*.o" -or -name "qrc_*" -or -name "Makefile*" -or -name "*.a" \) -exec rm {} \;
	fold_end

	fold_start "create_appimage"
	print_important "Creating an AppImage for MediaElch ${VERSION_NAME}"
	cp ${PROJECT_DIR}/desktop/MediaElch.png ${PROJECT_DIR}/
	./linuxdeployqt.AppImage ./desktop/MediaElch.desktop -verbose=1 -appimage
	fold_end

	fold_start "renaming"
	print_info "Renaming .AppImage"
	mv ${PROJECT_DIR}/MediaElch-${VERSION}*.AppImage ${PROJECT_DIR}/MediaElch_linux_${VERSION_NAME}.AppImage
	chmod +x ${PROJECT_DIR}/*.AppImage
	fold_end

	popd
}

create_macos_dmg() {
	print_important "Running macdeployqt"
	/usr/local/opt/qt/bin/macdeployqt MediaElch.app -dmg

	print_info "Renaming .dmg"
	mv "${PROJECT_DIR}/MediaElch.dmg" "${PROJECT_DIR}/MediaElch_macOS_${VERSION_NAME}.dmg"
}

package_zip() {
	print_info "package build into zip for win"

	print_info "Assembling package - Copying DLLs"
	mkdir -p pkg-zip/MediaElch
	cp "${PROJECT_DIR}/release/MediaElch.exe" pkg-zip/MediaElch/

	cat > "/tmp/dll_list.txt" <<EOF
qt5/bin/*.dll
bin/icudt56.dll
bin/icuin56.dll
bin/icuuc56.dll
bin/libbz2.dll
bin/libeay32.dll
bin/libfreetype-6.dll
bin/libgcc_s_sjlj-1.dll
bin/libglib-2.0-0.dll
bin/libharfbuzz-0.dll
bin/libiconv-2.dll
bin/libintl-8.dll
bin/libjpeg-9.dll
bin/libpcre16-0.dll
bin/libpcre-1.dll
bin/libpng16-16.dll
bin/libsqlite3-0.dll
bin/libstdc++-6.dll
bin/zlib1.dll
bin/ssleay32.dll
EOF

	local MXELIB=${MXEDIR}/usr/${MXETARGET}

	for file in $(cat /tmp/dll_list.txt); do
		cp ${MXEDIR}/usr/${MXETARGET}/${file} pkg-zip/MediaElch/
	done

	mkdir -p pkg-zip/MediaElch/sqldrivers
	cp ${MXELIB}/qt5/plugins/sqldrivers/qsqlite.dll                 pkg-zip/MediaElch/sqldrivers

	mkdir -p pkg-zip/MediaElch/platforms
	cp ${MXELIB}/qt5/plugins/platforms/qwindows.dll                 pkg-zip/MediaElch/platforms
	cp ${MXELIB}/qt5/plugins/platforms/qminimal.dll                 pkg-zip/MediaElch/platforms

	mkdir -p pkg-zip/MediaElch/QtQuick/Controls
	cp ${MXELIB}/qt5/qml/QtQuick/Controls/qmldir                    pkg-zip/MediaElch/QtQuick/Controls
	cp ${MXELIB}/qt5/qml/QtQuick/Controls/qtquickcontrolsplugin.dll pkg-zip/MediaElch/QtQuick/Controls

	cp -R ${MXELIB}/qt5/qml/QtQml/            pkg-zip/MediaElch/
	cp -R ${MXELIB}/qt5/qml/QtQuick.2/        pkg-zip/MediaElch/
	cp -R ${MXELIB}/qt5/plugins/imageformats/ pkg-zip/MediaElch/
	cp -R ${MXELIB}/qt5/plugins/mediaservice/ pkg-zip/MediaElch/

	fold_start "media_dll"
	print_info "Downloading MediaInfo.dll"
	wget --output-document MediaInfoDLL.7z https://mediaarea.net/download/binary/libmediainfo0/18.03.1/MediaInfo_DLL_18.03.1_Windows_x64_WithoutInstaller.7z
	7z x -oMediaInfo MediaInfoDLL.7z
	cp ./MediaInfo/MediaInfo.dll pkg-zip/MediaElch/
	fold_end

	fold_start "zipping_exe"
	print_info "zipping '${PROJECT_DIR}/MediaElch_win_${VERSION_NAME}.zip'"
	pushd pkg-zip > /dev/null
	zip -r "${PROJECT_DIR}/MediaElch_win_${VERSION_NAME}.zip" *
	popd
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
		"licenses": ["LGPL-3.0"]
	},
	"version": {
		"name": "${VERSION_NAME}",
		"released": "${RELEASE_DATE}",
		"gpgSign": false
	},
	"files":
	[
		{
			"includePattern": "${PROJECT_DIR}/MediaElch_${TARGET_OS}_${VERSION_NAME}.${FILE_TYPE}",
			"uploadPattern": "MediaElch_${TARGET_OS}_${VERSION_NAME}.${FILE_TYPE}"
		}
	],
	"publish": true
}
EOF
}

print_important "Packaging ${QT} (${OS_NAME}) for deployment"
pushd "${PROJECT_DIR}" > /dev/null

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
