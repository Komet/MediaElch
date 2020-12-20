#!/usr/bin/env bash

###########################################################
#
# CI - Install MediaElch dependencies for Linux
#
# This script installs all dependencies for MediaElch on
# linux (GCC).
#
# Linux builds can use different Qt versions. Set $QT_PPA
# to select a Qt version. All available versions are
# listed here: https://launchpad.net/~beineri/
#
###########################################################

set -Eeuo pipefail
IFS=$'\n\t'

# Go to project directory
cd "$(dirname "${BASH_SOURCE[0]}")/.." > /dev/null 2>&1
PROJECT_DIR="$(pwd)"

if [ -z ${QT+x} ]; then
	print_error "\$QT is unset"
	return 1
fi

#######################################################
# Globals
export MEDIAINFO_VERSION="20.03"

# Load utils (paths, color output, folding, etc.)
source "${PROJECT_DIR}/.ci/utils.sh"
source "${PROJECT_DIR}//scripts/utils.sh"

cd "${PROJECT_DIR}"

print_important "Getting dependencies for building for ${QT} on ${OS_NAME}"

fold_start "git_submodule"
print_info "Downloading quazip"
git submodule update --init -- third_party/quazip
fold_end

if [ -z "${QT_PPA+x}" ]; then
	print_error "\$QT_PPA is unset"
	print_error "For valid PPAs see https://launchpad.net/~beineri/"
	return 1
fi

#######################################################
# Repositories

fold_start "update"
print_info "Add repositories + update"
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo add-apt-repository -y ppa:beineri/opt-${QT_PPA}-bionic
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

print_important "Successfully installed dependencies"
