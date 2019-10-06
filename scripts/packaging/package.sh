#!/usr/bin/env bash

set -e          # Exit on errors
set -o pipefail # Unveils hidden failures

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
PROJECT_DIR="$( cd "${SCRIPT_DIR}/../.." ; pwd -P )"
PROJECT_DIR_NAME=${PROJECT_DIR##*/}
BUILD_DIR="${PROJECT_DIR}/build"
BUILD_OS=${1:-}
PACKAGE_TYPE=${2:-}
VERSION=

cd "${SCRIPT_DIR}"
source ../utils.sh
source check_dependencies.sh

if [ ! -f "/etc/debian_version" ]; then
	print_critical "Package script only works on Debian/Ubuntu systems!"
fi

print_help() {
	echo ""
	echo "Usage: ./package_release.sh (linux|win) package-type [options]"
	echo ""
	echo "This script builds and packages MediaElch in a distributable form."
	echo "It is used by MediaElch authors to release official MediaElch releases."
	echo ""
	echo "    DO NOT USE THIS IF YOU'RE AN END-USER"
	echo ""
	echo "Use build_release.sh if you want to build MediaElch."
	echo ""
	echo "Package Types:"
	echo "  linux"
	echo "    AppImage  Create a linux app image. See https://appimage.org/"
	echo "    deb       Build an unsigned deb package. Only for testing or"
	echo "              distributing test-branches."
	echo "    launchpad Upload a new MediaElch version to launchpad.net. You need a "
	echo "              clean repository for that (no build files)."
	echo "              Note: Set your signing key in $ME_SIGNING_KEY."
	echo "  win"
	echo "    zip       Create a ZIP file containing MediaElch and its dependencies."
	echo ""
	echo "Options"
	echo "  --no-confirm   Package MediaElch without confirm dialog."
	echo ""
}

# Gather information for packaging MediaElch such as version, git
# hash and date.
gather_information() {
	pushd "${PROJECT_DIR}" > /dev/null

	# Update or download in case something is outdated
	git submodule update --init

	VERSION_FULL=$(sed -ne 's/.*AppVersionFullStr[^"]*"\(.*\)";.*/\1/p' Version.h) # Format: 2.4.3-dev
	VERSION=${VERSION_FULL//-dev/}                                                 # Format: 2.4.3
	GIT_VERSION_FULL=$(git describe --abbrev=12 | sed -e 's/-g.*$// ; s/^v//')     # Format: 2.4.3-123
	GIT_VERSION=${GIT_VERSION_FULL//-/.}                                           # Format: 2.4.3
	GIT_REVISION=${GIT_VERSION_FULL//*-/}                                          # Format: 123
	GIT_DATE=$(git --git-dir=".git" show --no-patch --pretty="%ci")
	# RELEASE_DATE=$(date -u +"%Y-%m-%dT%H:%M:%S%z" --date="${GIT_DATE}")
	GIT_HASH=$(git --git-dir=".git" show --no-patch --pretty="%h")
	DATE_HASH=$(date -u +"%Y-%m-%d_%H-%M")
	VERSION_NAME="${VERSION}_${DATE_HASH}_git-${TRAVIS_BRANCH}-${GIT_HASH}"

	if [[ -z "$GIT_REVISION" ]] || [[ "$GIT_VERSION" == "$GIT_VERSION" ]]; then
		GIT_REVISION="1" # May be empty or equal to the current tag
	fi

	print_important "Information used for packaging:"
	echo "  Git Version         = ${GIT_VERSION}"
	echo "  Git Revision        = ${GIT_REVISION}"
	echo "  Git Date            = ${GIT_DATE}"
	#echo "  Release Date      = ${RELEASE_DATE}"
	echo "  Git Hash            = ${GIT_HASH}"
	echo "  Date Hash           = ${DATE_HASH}"
	echo "  MediaElch Version   = ${VERSION}"
	echo "  Version Name (Long) = ${VERSION_NAME}"

	if [[ ! "$GIT_VERSION" = "$VERSION" ]]; then
		echo ""
		print_error  "Git version and MediaElch version do not match!"
		print_error  "Add a new Git tag using:"
		print_error  "  git tag -a v1.1.0 -m \"Version 1.1.0\""
		echo         ""
		print_error  "Will still continue"
	fi

	popd > /dev/null
}

confirm_build() {
	echo ""
	print_important "Do you want to package MediaElch ${VERSION} for ${BUILD_OS} with these settings?"
	print_important "It is recommended to clean your repository using \"git clean -fdx\"."
	read -r -s -p  "Press enter to continue, Ctrl+C to cancel"
	echo ""
}

# Creates an .AppImage file that can be distributed.
package_appimage() {
	check_dependencies_linux_appimage

	if [ ! -d "${BUILD_DIR}" ] || [ ! -f "${BUILD_DIR}/.qmake.stash" ]; then
		$SCRIPT_DIR/../build_release.sh linux
	fi

	# sudo apt install qt5-style-plugins

	pushd "${BUILD_DIR}" > /dev/null
	echo ""
	print_important "Create an AppImage release"

	# Workaround for: https://github.com/probonopd/linuxdeployqt/issues/65
	unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH
	# linuxdeployqt uses $VERSION this for naming the file
	export VERSION;

	if [[ ! "$PATH" = *"qt"* ]] && [[ ! "$PATH" = *"Qt"* ]]; then
		print_critical "/path/to/qt/bin must be in your \$PATH, e.g. \nexport PATH=\"\$PATH:/usr/lib/x86_64-linux-gnu/qt5/bin\""
	fi

	if [[ "$(qmlimportscanner)" = *"could not find a Qt installation"* ]]; then
		print_critical "qmlimportscanner could not find a Qt installation.\nInstall qtdeclarative5-dev-tools\"."
	fi

	#######################################################
	# Download linuxdeployqt

	echo ""
	pushd "${PROJECT_DIR}" > /dev/null
	print_info "Downloading linuxdeployqt"
	if [ ! -f linuxdeploy-x86_64.AppImage ]; then
		wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
		chmod u+x linuxdeploy*.AppImage
	fi
	if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
		wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
		chmod u+x linuxdeploy*.AppImage
	fi
	popd > /dev/null

	#######################################################
	# Install MediaElch into subdirectory

	echo ""
	print_info "Installing MediaElch in subdirectory to create basic AppDir structure"
	make INSTALL_ROOT=appdir -j"$(nproc)" install
	tree appdir/

	#######################################################
	# Copy libmediainfo
	#
	# libmediainfo.so.0 is loaded at runtime that's why
	# linuxdeployqt can't detect it and we have to include
	# it here.

	echo ""
	print_info "Copying libmediainfo.so"
	mkdir -p ./appdir/usr/lib
	cp /usr/lib/x86_64-linux-gnu/libmediainfo.so.0 ./appdir/usr/lib/

	#######################################################
	# Download and copy ffmpeg

	echo ""
	if [ -f ffmpeg.tar.xz ]; then
		echo "Using existing ffmpeg"
	else
		print_info "Downloading ffmpeg"
		# Use static ffmpeg
		wget -c https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz -O ffmpeg.tar.xz
		ffmpeg_md5="d20e007536c6a7ab2ee63ea66c97472b  ffmpeg.tar.xz"
		if [ "$(md5sum ffmpeg.tar.xz)" = "${ffmpeg_md5}" ]; then
			print_info "FFMPEG MD5 checksum is valid"
		else
			print_error "MD5 checksum no valid"
			print_error "  Expected: ${ffmpeg_md5}"
			print_error "  Was:      $(md5sum ffmpeg.tar.xz)"
			exit 1
		fi
	fi
	tar -xJvf ffmpeg.tar.xz
	print_info "Copying ffmpeg into AppDir"
	cp ffmpeg-*/ffmpeg appdir/usr/bin/

	#######################################################
	# Create AppImage

	echo ""
	print_important "Creating an AppImage for MediaElch ${VERSION_NAME}. This takes a while and may seem frozen."
	export QML_SOURCES_PATHS="${PROJECT_DIR}/src/ui"
	export EXTRA_QT_PLUGINS="qt5dxcb-plugin"
	# Run linuxdeploy with following settings:
	# - use qt plugin       => so that Qt libraries are bundled correctly
	# - use appimage plugin => create an appimage file (essentially just bundles the appdir)
	"${PROJECT_DIR}/linuxdeploy-x86_64.AppImage" --appdir appdir       \
		--desktop-file appdir/usr/share/applications/MediaElch.desktop \
		--plugin qt                                                    \
		--output appimage
	find . -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq

	#######################################################
	# Finalize AppImage (name, chmod)

	echo ""
	print_info "Renaming .AppImage"
	mv ./MediaElch-${VERSION}*.AppImage MediaElch_linux_${VERSION_NAME}.AppImage
	chmod +x ./*.AppImage

	print_success "Successfully created AppImage: "
	print_success "    $(pwd)/MediaElch_linux_${VERSION_NAME}.AppImage"
	popd > /dev/null
}

prepare_deb() {
	echo ""
	check_dependencies_linux_deb

	cd "${PROJECT_DIR}/.."

	# For Debian packaging, we need a distinct version for each upload
	# to launchpad. We can achieve this by using the git revision.
	VERSION="${VERSION}.${GIT_REVISION}"

	# Create target directory
	TARGET_DIR=mediaelch-${VERSION}
	rm -rf ${TARGET_DIR} && mkdir ${TARGET_DIR}

	print_info "Copying sources to ./${TARGET_DIR}"
	( cd $PROJECT_DIR_NAME; tar cf - . ) | (cd ${TARGET_DIR}; tar xf - )

	pushd ${TARGET_DIR}  > /dev/null

	# Remove untracked files but keep changes
	git clean -fdx
	# .git is ~20MB, so remove it
	rm -rf scripts obs travis-ci docs .git

	# A bit of useful information
	echo $GIT_HASH > .githash
	echo $GIT_DATE > .gitdate
	echo $GIT_VERSION > .gitversion

	# New Version; Get rivision that is not in changelog
	PPA_REVISION=0
	while [ $PPA_REVISION -le "99" ]; do
		PPA_REVISION=$((PPA_REVISION+1))
		if [[ ! $(grep $VERSION-$PPA_REVISION debian/changelog) ]]; then
			break
		fi
	done

	# Create changelog entry
	print_info "Adding new entry for version ${VERSION}-${PPA_REVISION} in"
	print_info "debian/changelog using information from debian/control"
	dch -v "$VERSION-$PPA_REVISION~xenial" -D xenial -M -m "next build"
	cp debian/changelog "${PROJECT_DIR}/debian/changelog"

	popd > /dev/null

	print_info "Create source file (.tar.gz)"
	tar ch "${TARGET_DIR}" | xz > "mediaelch_${VERSION}.orig.tar.xz"
}

# Creates an unsigned deb.
package_deb() {
	prepare_deb

	print_info "Run debuild"
	pushd "${TARGET_DIR}" > /dev/null
	debuild -uc -us
	popd > /dev/null
}

# Uploads a new MediaElch release to launchpad.net
package_and_upload_to_launchpad() {
	echo ""
	print_important "Upload a new MediaElch Version to https://launchpad.net"

	if [ -z "$ME_LAUNCHPAD_TYPE" ]; then
		print_error "\$ME_LAUNCHPAD_TYPE is not set or empty! Can be either stable/nightly/test"
		exit 1
	fi

	print_important "\$ME_LAUNCHPAD_TYPE is: ${ME_LAUNCHPAD_TYPE} (can be either stable/nightly/test)"
	print_important "Is this correct?"
	[ "${no_confirm}" != "--no-confirm" ] && read -r -s -p  "Press enter to continue, Ctrl+C to cancel"

	if [ -z "$ME_SIGNING_KEY" ]; then
		print_critical "\$ME_SIGNING_KEY is empty or not set! Must be a GPG key id"
	fi

	prepare_deb

	print_info "Run debuild"
	pushd ${TARGET_DIR} > /dev/null
	debuild -k${ME_SIGNING_KEY} -S

	# Create builds for other Ubuntu releases that Launchpad supports
	distr=xenial # Ubuntu 16.04
	others="bionic disco" # Ubuntu 17.10 and 19.04
	for next in $others
	do
		sed -i "s/${distr}/${next}/g" debian/changelog
		debuild -k${ME_SIGNING_KEY} -S
		distr=$next
	done

	popd > /dev/null

	pushd "${PROJECT_DIR}/.." > /dev/null
	dput ppa:mediaelch/mediaelch-${ME_LAUNCHPAD_TYPE} mediaelch_${VERSION}-${PPA_REVISION}~*.changes
	popd > /dev/null
}

pkg_type="$(lc "${PACKAGE_TYPE:-invalid}")"
no_confirm=${3:-confirm}

if [ "${BUILD_OS}" == "linux" ] ; then
	if [ "$pkg_type" != "appimage" ] && [ "$pkg_type" != "deb" ] && [ "$pkg_type" != "launchpad" ]; then
		print_error "Unknown package type for linux: \"${PACKAGE_TYPE}\""
		print_help
		exit 1
	fi

	gather_information
	[ "${no_confirm}" != "--no-confirm" ] && confirm_build
	echo ""

	if [ "$pkg_type" = "appimage" ]; then
		package_appimage ${no_confirm}
	elif [ "$pkg_type" = "launchpad" ]; then
		package_and_upload_to_launchpad
	elif [ "$pkg_type" = "deb" ]; then
		package_deb
	fi

elif [ "${BUILD_OS}" == "win" ]; then
	print_info "Windows build not supported, yet!"
	exit 1

else
	print_error "Unknown OS \"${BUILD_OS}\""
	print_help
	exit 1
fi

