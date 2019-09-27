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

#######################################################
# Globals
export MEDIAINFO_VERSION="19.04"

# Load utils (paths, color output, folding, etc.)
source "${SCRIPT_DIR}/utils.sh"
source "${SCRIPT_DIR}/../scripts/utils.sh"

cd "${PROJECT_DIR}"

print_important "Getting dependencies for building for ${QT} on ${OS_NAME}"

fold_start "git_submodule"
print_info "Downloading quazip"
git submodule update --init -- third_party/quazip
fold_end

if [ "$(lc "${OS_NAME}")" = "linux" ]; then

	if [ $QT = "qtWin" ]; then
		export MXEDIR="/usr/lib/mxe"
		export MXEINFIX="x86-64-w64-mingw32.shared"
		export MXETARGET="x86_64-w64-mingw32.shared"

		# defs.sh is read by "configure.sh"
		{
			echo "#!/usr/bin/env bash"
			echo "MXEDIR=\"${MXEDIR}\""
			echo "MXEINFIX=\"${MXEINFIX}\""
			echo "MXETARGET=\"${MXETARGET}\""
		} > ${SCRIPT_DIR}/defs.sh

		#######################################################
		# Repositories

		fold_start "mxe_repo"
		print_info "Adding mirror.mxe.cc apt repo"
		sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 86B72ED9
		sudo add-apt-repository 'deb [arch=amd64] https://mirror.mxe.cc/repos/apt xenial main'
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
		sudo apt-get install -y mxe-${MXEINFIX}-gcc \
			mxe-${MXEINFIX}-gnutls \
			mxe-${MXEINFIX}-zlib \
			mxe-${MXEINFIX}-qtbase \
			mxe-${MXEINFIX}-qttools \
			mxe-${MXEINFIX}-qtmultimedia \
			mxe-${MXEINFIX}-qtimageformats \
			mxe-${MXEINFIX}-qtquickcontrols
		fold_end

		echo "Make MXE writable"
		sudo chmod -R a+w "${MXEDIR}"

		#######################################################
		# MediaInfoDLL & ZenLib

		fold_start "mediainfodll"
		print_info "Downloading MediaInfoDLL"
		wget --output-document MediaInfoDLL.7z https://mediaarea.net/download/binary/libmediainfo0/${MEDIAINFO_VERSION}/MediaInfo_DLL_${MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z
		7zr x -oMediaInfo MediaInfoDLL.7z
		mv MediaInfo/Developers/Source/MediaInfoDLL ./MediaInfoDLL
		mv MediaInfo/MediaInfo.dll MediaInfo.dll
		rm -rf MediaInfo MediaInfoDLL.7z
		fold_end

		fold_start "zenlib"
		print_info "Downloading ZenLib"
		svn checkout https://github.com/MediaArea/ZenLib/trunk/Source/ZenLib ./ZenLib
		fold_end

	else

		if [ -z "${QT_PPA+x}" ]; then
			print_error "\$QT_PPA is unset";
			print_error "For valid PPAs see https://launchpad.net/~beineri/"
			return 1;
		fi

		#######################################################
		# Repositories

		fold_start "update"
		print_info "Add repositories + update"
		sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
		sudo add-apt-repository -y ppa:beineri/opt-${QT_PPA}-xenial
		sudo apt-get -qq update
		fold_end

		#######################################################
		# Compiler & Build System

		fold_start "update_compiler"
		print_info "Installing CMake using pip"
		pip install --user cmake
		print_info "Updating GCC"
		sudo apt-get install -y g++-8 gcc-8
		sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 90
		sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 90
		fold_end

		#######################################################
		# Dependencies

		fold_start "install_qt"
		print_info "Installing Python3 and pip3"
		sudo apt install python3-setuptools python3-pip
		fold_end

		fold_start "install_qt"
		print_info "Installing Qt packages"
		sudo apt-get install -y ${QT}base ${QT}multimedia ${QT}declarative ${QT}quickcontrols
		fold_end

		fold_start "install_other"
		print_info "Installing other dependencies"
		sudo apt-get install -y \
			libcurl4-openssl-dev libmediainfo-dev \
			libpulse-dev zlib1g-dev libzen-dev \
			libgl1-mesa-dev
		fold_end
	fi

else
	print_error "Unknown operating system: ${OS_NAME}"
	exit 1
fi

print_important "Successfully installed dependencies"
