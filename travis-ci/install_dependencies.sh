#!/usr/bin/env bash

###########################################################
#
# Travis CI - Install MediaElch dependencies
#
# This script installs all dependencies for MediaElch on
# linux (GCC) and macOS (clang) and windows (MXE).
#
# Linux builds can use different Qt versions. Set $QT_PPA
# to select a Qt version. All available versions are
# listed here: https://launchpad.net/~beineri/
# macOS builds use the latest Qt version available with
# Homebrew (https://brew.sh).
# Windows builds use the latest Qt version available
# with mxe.
#
###########################################################

# Exit on errors
set -e

if [ -z ${QT+x} ]; then print_error "\$QT is unset"; return 1; fi

SCRIPT_DIR="$( cd "$(dirname "$0")" ; pwd -P )"

# Load utils (paths, color output, folding, etc.)
. "${SCRIPT_DIR}/utils.sh"

pushd "${PROJECT_DIR}" > /dev/null

print_important "Getting dependencies for building for ${QT} on ${OS_NAME}"

if [ $(lc "${OS_NAME}") = "linux" ]; then

	if [ $QT = "qtWin" ]; then
		MXEDIR="/usr/lib/mxe"
		MXETARGET="i686-w64-mingw32.shared"

		# defs.sh is read by "configure.sh"
		echo "#!/usr/bin/env bash"        >  ${SCRIPT_DIR}/defs.sh
		echo "MXEDIR=\"${MXEDIR}\""       >> ${SCRIPT_DIR}/defs.sh
		echo "MXETARGET=\"${MXETARGET}\"" >> ${SCRIPT_DIR}/defs.sh

		#######################################################
		# Repositories

		fold_start "mxe_repo"
		print_info "Adding pkg.mxe.cc apt repo"
		echo "deb http://pkg.mxe.cc/repos/apt/debian wheezy main" | sudo tee /etc/apt/sources.list.d/mxeapt.list > /dev/null
		sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys D43A795B73B16ABE9643FE1AFD8FFF16DB45C6AB
		fold_end

		fold_start "update"
		print_info "Updating apt cache"
		sudo apt-get -qq update
		fold_end

		# Needed to extract MediaInfoDLL
		sudo apt install -y p7zip

		#######################################################
		# MXE Dependencies

		fold_start "mxe_install"
		print_info "Installing mxe"
		sudo apt-get install -y mxe-${MXETARGET}-gcc \
			mxe-${MXETARGET}-qtbase \
			mxe-${MXETARGET}-qttools \
			mxe-${MXETARGET}-qtscript \
			mxe-${MXETARGET}-qtmultimedia \
			mxe-${MXETARGET}-qtimageformats \
			mxe-${MXETARGET}-qtquickcontrols
		fold_end

		echo "Make MXE writable"
		sudo chmod -R a+w "${MXEDIR}"

		#######################################################
		# MediaInfoDLL & ZenLib

		fold_start "mediainfo"
		print_info "Downloading MediaInfoDLL and ZenLib headers"
		svn checkout https://github.com/MediaArea/MediaInfoLib/trunk/Source/MediaInfoDLL ./MediaInfoDLL
		svn checkout https://github.com/MediaArea/ZenLib/trunk/Source/ZenLib ./ZenLib
		fold_end

	else

		if [ -z ${QT_PPA+x} ]; then
			print_error "\$QT_PPA is unset";
			print_error "For valid PPAs see https://launchpad.net/~beineri/"
			return 1;
		fi

		#######################################################
		# Repositories

		fold_start "update"
		print_info "Add repositories + update"
		sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
		sudo add-apt-repository -y ppa:beineri/opt-${QT_PPA}-trusty
		sudo apt-get -qq update
		fold_end

		#######################################################
		# Compiler

		fold_start "update_compiler"
		print_info "Updating GCC"
		sudo apt-get install -y g++-7 gcc-7
		sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 90
		sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 90
		fold_end

		#######################################################
		# Dependencies

		fold_start "install_qt"
		print_info "Installing Qt packages"
		sudo apt-get install -y ${QT}base ${QT}script ${QT}multimedia ${QT}declarative
		fold_end

		fold_start "install_other"
		print_info "Installing other dependencies"
		sudo apt-get install -y libcurl4-openssl-dev libmediainfo-dev libpulse-dev zlib1g-dev libzen-dev
		fold_end
	fi

elif [ "${OS_NAME}" = "Darwin" ]; then

	fold_start "download_libraries"
	print_info "Dowloading MediaInfoLib sources"
	svn checkout https://github.com/MediaArea/MediaInfoLib/trunk/Source/MediaInfoDLL

	print_info "Dowloading ZenLib sources"
	svn checkout https://github.com/MediaArea/ZenLib/trunk/Source/ZenLib

	print_info "Updating homebrew"
	brew update > brew_update.log || {
		print_error "Updating homebrew failed. Error log:";
		cat brew_update.log;
		exit 1;
	}
	print_info "Brewing packages: qt5 media-info"
	brew install qt5 media-info
	fold_end

else
	print_error "Unknown operating system: ${OS_NAME}"
	exit 1
fi

print_important "Successfully installed dependencies"
popd > /dev/null
