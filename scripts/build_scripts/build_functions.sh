#!/usr/bin/env bash

#
# Functions for building MediaElch.
# Source this script and make sure to have utils.sh loaded
#

set -e

build_release_linux() {
	if [ "${BUILD_DIR}" == "" ]; then
		echo "\$BUILD_DIR not set."
		exit 1
	fi

	rm -rf "${BUILD_DIR}"
	mkdir -p "${BUILD_DIR}"

	pushd ${BUILD_DIR} > /dev/null
	print_important "Running qmake"
	qmake "${PROJECT_DIR}/MediaElch.pro" CONFIG+=release
	echo ""

	print_important "Building MediaElch (only warnings and errors shown)"
	make -j $(nproc) 1>/dev/null
	popd > /dev/null
}
