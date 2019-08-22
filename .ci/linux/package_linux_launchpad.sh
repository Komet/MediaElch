#!/usr/bin/env bash

# TODO
# Refactor and put deb creation into custom shell script.

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/../.." > /dev/null 2>&1

#######################################################
# Variables & Functions

PROJECT_DIR="$(pwd -P)"
PROJECT_DIR_NAME=${PROJECT_DIR##*/}
PACKAGE_TYPE=${1:-}

source .ci/ci_utils.sh

if [ ! -f "/etc/debian_version" ]; then
	print_fatal "Package script only works on Debian/Ubuntu systems!"
fi

print_help() {
	cat << EOF
Usage: $(basename "$0") <package-type> [options]

This script builds and packages MediaElch and uploads it to launchpad.net.
It is used by MediaElch authors to release official MediaElch releases.

    DO NOT USE THIS IF YOU'RE AN END-USER

Use .ci/linux/build_linux_release.sh if you want to build MediaElch.

Package Types:
  linux
    deb       Build an unsigned deb package. Only for testing or
              distributing test-branches.
    launchpad Upload a new MediaElch version to launchpad.net. You need a
              clean repository for that (no build files).
              Note: Set your signing key in $ME_SIGNING_KEY.

Options
  --no-confirm   Package MediaElch without confirm dialog.
EOF
	exit 1
}

confirm_build() {
	echo ""
	print_important "Do you want to package MediaElch ${ME_VERSION} for Linux with these settings?"
	print_important "It is recommended to clean your repository using \"git clean -fdx\"."
	read -r -s -p "Press enter to continue, Ctrl+C to cancel"
	echo ""
}

#######################################################
# Getting Details

# Check for linux build and packaging dependencies
source .ci/linux/check_linux_dependencies.sh
check_dependencies_linux
check_dependencies_linux_deb

# Exports required variables
gather_project_and_system_details

#######################################################
# Functions

prepare_deb() {
	cd "${PROJECT_DIR}/.."

	# For Debian packaging, we need a distinct version for each upload
	# to launchpad. We can achieve this by using the git revision.
	ME_VERSION="${ME_VERSION}.${GIT_REVISION}"

	# Create target directory
	TARGET_DIR=mediaelch-${ME_VERSION}
	rm -rf ${TARGET_DIR} && mkdir ${TARGET_DIR}

	print_info "Copying sources to ./${TARGET_DIR}"
	(
		cd $PROJECT_DIR_NAME
		tar cf - .
	) | (
		cd ${TARGET_DIR}
		tar xf -
	)

	pushd ${TARGET_DIR} > /dev/null

	# Remove untracked files but keep changes
	git clean -fdx
	# .git is ~20MB, so remove it
	rm -rf scripts obs .ci docs .git

	# A bit of useful information
	echo $GIT_HASH > .githash
	echo $GIT_DATE > .gitdate
	echo $GIT_VERSION > .gitversion

	# New Version; Get rivision that is not in changelog
	PPA_REVISION=0
	while [ $PPA_REVISION -le "99" ]; do
		PPA_REVISION=$((PPA_REVISION + 1))
		if [[ ! $(grep $ME_VERSION-$PPA_REVISION debian/changelog) ]]; then
			break
		fi
	done

	# Create changelog entry
	print_info "Adding new entry for version ${ME_VERSION}-${PPA_REVISION} in"
	print_info "debian/changelog using information from debian/control"
	dch -v "$ME_VERSION-$PPA_REVISION~bionic" -D bionic -M -m "next build"
	cp debian/changelog "${PROJECT_DIR}/debian/changelog"

	popd > /dev/null

	print_info "Create source file (.tar.gz)"
	tar ch "${TARGET_DIR}" | xz > "mediaelch_${ME_VERSION}.orig.tar.xz"
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
	[ "${no_confirm}" != "--no-confirm" ] && read -r -s -p "Press enter to continue, Ctrl+C to cancel"

	if [ -z "$ME_SIGNING_KEY" ]; then
		print_fatal "\$ME_SIGNING_KEY is empty or not set! Must be a GPG key id"
	fi

	prepare_deb

	print_info "Run debuild"
	pushd ${TARGET_DIR} > /dev/null
	debuild -k${ME_SIGNING_KEY} -S

	# Create builds for other Ubuntu releases that Launchpad supports
	distr=bionic          # Ubuntu 18.04
	others="focal groovy" # Ubuntu 20.04, 20.10
	for next in $others; do
		sed -i "s/${distr}/${next}/g" debian/changelog
		debuild -k${ME_SIGNING_KEY} -S
		distr=$next
	done

	popd > /dev/null

	pushd "${PROJECT_DIR}/.." > /dev/null
	dput ppa:mediaelch/mediaelch-${ME_LAUNCHPAD_TYPE} mediaelch_${ME_VERSION}-${PPA_REVISION}~*.changes
	popd > /dev/null
}

pkg_type="$(lc "${PACKAGE_TYPE:-invalid}")"
no_confirm=${2:-confirm}

if [ "$pkg_type" != "deb" ] && [ "$pkg_type" != "launchpad" ]; then
	print_error "Unknown package type for linux: \"${PACKAGE_TYPE}\""
	print_help
	exit 1
fi

[ "${no_confirm}" != "--no-confirm" ] && confirm_build
echo ""

if [ "$pkg_type" = "launchpad" ]; then
	package_and_upload_to_launchpad
elif [ "$pkg_type" = "deb" ]; then
	package_deb
fi
