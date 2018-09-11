#!/usr/bin/env bash

#
# Functions for packaging MediaElch.
# Source this script and make sure to have utils.sh loaded
#

set -e

# Creates an .AppImage file that can be distributed.
package_appimage() {
	check_dependencies_linux_appimage

	$SCRIPT_DIR/build_release.sh linux

	pushd "${BUILD_DIR}" > /dev/null
	echo ""
	print_important "Create an AppImage release"

	# Workaround for: https://github.com/probonopd/linuxdeployqt/issues/65
	unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH
	# linuxdeployqt uses this for naming the file
	export VERSION=${ME_VERSION}

	if [[ ! "$PATH" = *"qt"* ]]; then
		print_critical "/path/to/qt/bin must be in your \$PATH, e.g. \nexport PATH=\"\$PATH:/usr/lib/x86_64-linux-gnu/qt5/bin\""
	fi

	if [[ "$(qmlimportscanner)" = *"could not find a Qt installation"* ]]; then
		print_critical "qmlimportscanner could not find a Qt installation.\nInstall qtdeclarative5-dev-tools\"."
	fi

	echo ""
	print_info "Downloading linuxdeployqt"
	DEPLOYQT="${PROJECT_DIR}/linuxdeployqt.AppImage"
	if [ ! -f $DEPLOYQT ]; then
		wget --output-document "${PROJECT_DIR}/linuxdeployqt.AppImage" \
			https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
	fi
	chmod u+x $DEPLOYQT

	echo ""
	print_info "Installing MediaElch in subdirectory to create basic AppDir structure"
	make INSTALL_ROOT=appdir -j $(nproc) install

	echo ""
	print_info "Copying ffmpeg into AppDir"
	cp $(which ffmpeg) appdir/usr/bin/

	echo ""
	print_important "Creating an AppImage for MediaElch ${VERSION_NAME}. This takes a while and may seem frozen."
	$DEPLOYQT appdir/usr/share/applications/MediaElch.desktop -verbose=1 -bundle-non-qt-libs -qmldir=../src/ui
	$DEPLOYQT --appimage-extract
	export PATH=$(readlink -f ./squashfs-root/usr/bin):$PATH
	# Workaround to increase compatibility with older systems
	# see https://github.com/darealshinji/AppImageKit-checkrt for details
	mkdir -p appdir/usr/optional/libstdc++/
	wget -c https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/exec-x86_64.so -O ./appdir/usr/optional/exec.so
	cp /usr/lib/x86_64-linux-gnu/libstdc++.so.6 ./appdir/usr/optional/libstdc++/
	rm appdir/AppRun
	wget -c https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64 -O appdir/AppRun
	chmod a+x appdir/AppRun
	./squashfs-root/usr/bin/appimagetool -g ./appdir/ MediaElch-${VERSION}-x86_64.AppImage

	echo ""
	print_info "Renaming .AppImage"
	mv MediaElch-${VERSION}*.AppImage MediaElch_linux_${VERSION_NAME}.AppImage
	chmod +x *.AppImage

	print_success "Successfully created AppImage: "
	print_success "    $(pwd)/MediaElch_linux_${VERSION_NAME}.AppImage"
	popd > /dev/null
}

# Creates an unsigned deb.
package_deb() {
	echo ""

	if [ -z "$DEBFULLNAME" ]; then
		print_critical "\$DEBFULLNAME is empty or not set!"
	fi
	if [ -z "$DEBEMAIL" ]; then
		print_critical "\$DEBEMAIL is empty or not set!"
	fi

	check_dependencies_linux_deb

	local FOLDER_NAME=${PROJECT_DIR##*/}
	pushd "${PROJECT_DIR}/.." > /dev/null

	cp -R ./${FOLDER_NAME} ./mediaelch-${ME_VERSION}

	tar --exclude="mediaelch-${ME_VERSION}.orig/scripts" --exclude="mediaelch-${ME_VERSION}/build" \
			--exclude=".git" --exclude="*.AppImage" \
			-cf "mediaelch_${ME_VERSION}.orig.tar" "${FOLDER_NAME}"
	gzip "mediaelch_${ME_VERSION}.orig.tar"

	pushd "mediaelch-${ME_VERSION}" > /dev/null
	echo $PWD;
	debuild -uc -us
	popd > /dev/null
	popd > /dev/null
}

# Uploads a new MediaElch release to launchpad.net
package_and_upload_to_launchpad() {
	echo ""
	print_important "Upload a new MediaElch Version to https://launchpad.net"

	if [ -z "$DEBFULLNAME" ]; then
		print_critical "\$DEBFULLNAME is empty or not set!"
	fi
	if [ -z "$DEBEMAIL" ]; then
		print_critical "\$DEBEMAIL is empty or not set!"
	fi

	check_dependencies_linux_deb

	# TODO: Better put in README or doc
	pushd "${PROJECT_DIR}" > /dev/null
	print_important "Update changelog using dch"
	dch --newversion "${ME_VERSION}-1" --distribution vivid
	popd > /dev/null

	local FOLDER_NAME=${PROJECT_DIR##*/}
	pushd "${PROJECT_DIR}/.." > /dev/null

	cp -R ./${FOLDER_NAME} ./mediaelch-${ME_VERSION}

	tar --exclude="mediaelch-${ME_VERSION}.orig/scripts" --exclude="mediaelch-${ME_VERSION}/build" \
			--exclude=".git" --exclude="*.AppImage" \
			-cf "mediaelch_${ME_VERSION}.orig.tar" "${FOLDER_NAME}"
	gzip "mediaelch_${ME_VERSION}.orig.tar"

	pushd "mediaelch-${ME_VERSION}" > /dev/null
	echo $PWD;
	debuild -S -sa
	popd > /dev/null

	dput ppa:mediaelch/mediaelch mediaelch_${ME_VERSION}-1_source.changes
	popd > /dev/null
}