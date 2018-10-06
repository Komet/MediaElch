#!/usr/bin/env bash

#
# Functions for packaging MediaElch.
# Source this script and make sure to have utils.sh loaded
#

set -e

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
