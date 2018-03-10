#!/usr/bin/env sh

###########################################################
#
# Travis CI - Install MediaElch dependencies
#
# This script installs all dependencies for MediaElch on
# linux (GCC or clang) and macOS (clang).
#
# Linux builds can use different Qt versions. Set $QT_PPA
# to select a Qt version. For available versions see
# https://launchpad.net/~beineri/
# macOS builds use the latest Qt version available with
# Homebrew (https://brew.sh).
#
###########################################################

# Exit on errors
set -e

if [ -z ${QT+x} ]; then print_error "\$QT is unset"; return 1; fi

# Load utils (color output, folding, etc.)
. "${TRAVIS_BUILD_DIR}/travisCI/utils.sh"
cd "${TRAVIS_BUILD_DIR}"

print_important "Getting dependencies for building for ${QT} on ${TRAVIS_OS_NAME}"

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then

	if [ -z ${QT_PPA+x} ]; then
		print_error "\$QT_PPA is unset";
		print_error "For valid PPAs see https://launchpad.net/~beineri/"
		return 1;
	fi

	#######################################################
	# Repositories

	print_info "Add repositories + update"
	fold_start "update"
	sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
	sudo add-apt-repository -y ppa:beineri/opt-${QT_PPA}-trusty
	if [ "${CXX}" = "clang++" ]; then
		wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
		sudo apt-add-repository "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-6.0 main"
	fi
	sudo apt-get -qq update
	fold_end "update"

	#######################################################
	# Compilers

	print_info "Updating compiler \"${CXX}\"..."
	fold_start "update_compiler"
	if [ "${CXX}" = "g++" ]; then
		sudo apt-get install -y g++-7 gcc-7
		# Overwrite defaults
		sudo ln -sf ${BIN_DIR}/g++-7 ${BIN_DIR}/g++
		sudo ln -sf ${BIN_DIR}/gcc-7 ${BIN_DIR}/gcc

	elif [ "${CXX}" = "clang++" ]; then
		sudo apt-get install -y clang++-6.0 clang-6.0
		# Overwrite defaults
		sudo ln -s ${BIN_DIR}/clang++-6.0 ${BIN_DIR}/clang++
		sudo ln -s ${BIN_DIR}/clang-6.0 ${BIN_DIR}/clang

	else
		print_error "Unknown compiler."
		exit 1;
	fi
	fold_end "update_compiler"

	#######################################################
	# Dependencies

	print_info "Installing Qt packages"
	fold_start "qt_install"
	sudo apt-get install -y ${QT}base ${QT}script ${QT}multimedia ${QT}declarative
	fold_end "qt_install"

	print_info "Installing other dependencies"
	fold_start "other_install"
	sudo apt-get install -y libcurl4-openssl-dev libmediainfo-dev libpulse-dev zlib1g-dev libzen-dev
	fold_end "other_install"

elif [ "${TRAVIS_OS_NAME}" = "osx" ]; then

	print_info "Dowload MediaInfoLib sources"
	svn checkout https://github.com/MediaArea/MediaInfoLib/trunk/Source/MediaInfoDLL

	print_info "Dowload ZenLib sources"
	svn checkout https://github.com/MediaArea/ZenLib/trunk/Source/ZenLib

	print_info "Updating homebrew"
	brew update > brew_update.log || {
		print_error "Updating homebrew failed. Error log:";
		cat brew_update.log;
		exit 1;
	}
	print_info "Brewing packages: qt5 media-info"
	brew install qt5 media-info

else
	print_error "Unknown operating system."
	exit 1;
fi

print_important "Successfully installed dependencies"
cd "${TRAVIS_BUILD_DIR}"
