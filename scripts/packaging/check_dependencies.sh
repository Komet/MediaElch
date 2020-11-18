#!/usr/bin/env bash

#
# Functions for checking MediaElch dependencies.
# Source this script and make sure to have utils.sh loaded
#

HAS_MISSING=0

require_command() {
	if command -v "$1" >/dev/null 2>&1; then
		# shellcheck disable=SC2059
		printf "  ${GREEN}✔${NC} $1 installed\n"
	else
		# shellcheck disable=SC2059
		printf "  ${RED}✘${NC} $1 not installed.\n"
		HAS_MISSING=1
	fi
}

require_package_apt() {
	if [ "$(dpkg-query -W -f='${Status}' $1 2>/dev/null | grep -c "ok installed")" -eq 0 ]; then
		# shellcheck disable=SC2059
		printf "  ${RED}✘${NC} $1 not installed.\n"
		HAS_MISSING=1
	else
		# shellcheck disable=SC2059
		printf "  ${GREEN}✔${NC} $1 installed\n"
	fi
}

check_dependencies_linux() {
	HAS_MISSING=0

	print_important "Checking dependencies for Linux:"
	echo ""

	require_command gcc
	require_command g++
	require_command qmake

	echo ""
	print_important "Checking package dependencies for Linux:"
	echo ""

	require_package_apt libcurl3-gnutls # or libcurl4-openssl-dev
	require_package_apt libmediainfo-dev
	require_package_apt libpulse-dev
	require_package_apt zlib1g-dev
	require_package_apt libzen-dev

	echo ""

	if [ $HAS_MISSING -eq 0 ]; then
		print_success "All dependencies installed."
	else
		print_critical "Found missing dependencies. Abort."
	fi
}

check_dependencies_linux_appimage() {
	HAS_MISSING=0

	print_important "Checking dependencies for Linux AppImage:"
	echo ""

	require_command ffmpeg

	echo ""

	if [ $HAS_MISSING -eq 0 ]; then
		print_success "All dependencies installed."
	else
		print_critical "Found missing dependencies. Abort."
	fi
}

check_dependencies_linux_deb() {
	HAS_MISSING=0

	print_important "Checking dependencies for Debian package:"
	echo ""

	require_command tar
	require_command gzip
	require_command xz
	require_command debuild
	require_command dput

	echo ""

	if [ $HAS_MISSING -eq 0 ]; then
		print_success "All dependencies installed."
	else
		print_critical "Found missing dependencies. Abort."
	fi
}

print_system_info_unix() {
	echo ""
	print_important "System Information:"
	echo "  Architecture:    ${OS_REV}"
	echo "  Machine:         ${OS_MACH}"
	echo "  Concurrent Jobs: ${JOBS}"
	echo ""
}

check_dependencies_macOS() {
	HAS_MISSING=0

	print_important "Checking dependencies for macOS:"
	echo ""

	require_command git
	require_command qmake
	require_command clang++
	require_command svn
	require_command tar
	require_command wget
	require_command 7za
	require_command macdeployqt

	echo ""

	if [ $HAS_MISSING -eq 0 ]; then
		print_success "All dependencies installed."
	else
		print_critical "Found missing dependencies. Abort."
	fi
}