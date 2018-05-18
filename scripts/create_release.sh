#!/bin/bash
set -e

FullExecPath=$(pwd)
pushd $(dirname $0) > /dev/null
FullScriptPath=$(pwd)
popd > /dev/null
pushd ${FullScriptPath}/.. > /dev/null
FullSrcPath=$(pwd)
popd > /dev/null

AppVersionStrMajor=2.4
AppVersionStrFull=2.4.3-dev

BuildTarget=$1
TargetType=$2
ReleasePath="${FullSrcPath}/out/release/${BuildTarget}"
DeployPath="${FullSrcPath}/out/deploy/${BuildTarget}/${TargetType}/${AppVersionStrFull}"

JOBS=$(grep '^processor' /proc/cpuinfo | wc -l)

source ${FullScriptPath}/utils.sh

print_help() {
	echo ""
	echo " This script can build MediaElch for:"
	echo "  - Linux (AppImage)"
	echo "  - Windows"
	echo "  - macOS (needs to be run on a Mac)"
	echo ""
	echo " Usage:"
	echo "   ./scrips/build_release.sh target [type]"
	echo ""
	echo "     target:       linux|win|mac"
	echo "     type: linux:  AppImage|deb"
	echo "           mac:    dmg"
	echo "           win:    exe"
	echo ""
}

print_help_windows() {
	echo ""
	echo " To build for windows, you need mxe."
	echo " See: http://pkg.mxe.cc/"
	echo ""
	echo " You need to install following packages:"
	echo ""
	echo "   apt-get install -y mxe-i686-w64-mingw32.static-gcc \\"
	echo "       mxe-i686-w64-mingw32.static-qtbase \\"
	echo "       mxe-i686-w64-mingw32.static-qttools \\"
	echo "       mxe-i686-w64-mingw32.static-qtscript \\"
	echo "       mxe-i686-w64-mingw32.static-qtmultimedia \\"
	echo "       mxe-i686-w64-mingw32.static-qtdeclarative"
	echo ""
}

build_linux() {
	msg_important "Building version $AppVersionStrFull for Linux..."

	mkdir -p "${ReleasePath}"
	pushd "${ReleasePath}"

	qmake ${FullSrcPath}/MediaElch.pro -spec linux-g++
	make -j${JOBS} 1> /dev/null; # Print only warnings/errors
	msg_info "MediaElch build for linux complete."

	popd
}

# See https://github.com/probonopd/linuxdeployqt
# $1: Where to download linuxdeployqt to (if it does not exist)
deploy_appimage_setup() {
	# Download linuxdeployqt if it does not exist.
	if [ ! -f ${1} ]; then
		msg_info "Downloading linuxdeployqt"
		wget --output-document "${1}" https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
	else
		msg_info "linuxdeployqt already exists"
	fi
	chmod +x ${1}
}

deploy_appimage() {
	build_linux

	msg_important "Create an AppImage release"
	local DeployQt=${FullSrcPath}/linuxdeployqt.AppImage
	deploy_appimage_setup ${DeployQt}

	mkdir -p "${DeployPath}"
	pushd ${DeployPath} > /dev/null
	msg_important "Creating an AppImage for MediaElch ${AppVersionStrFull}..."
	cp ${FullSrcPath}/desktop/* ${ReleasePath}/ # Copy icon and desktop file
	${DeployQt} ${ReleasePath}/MediaElch -verbose=1 -appimage
	popd > /dev/null

	msg_info "AppImage release created successfully!"
}

deploy_deb_package() {
	#
	# apt install git-pbuilder git-buildpackage
	#
	mkdir -p ${ReleasePath}/tarballs
	git archive --prefix mediaelch/ -o ${ReleasePath}/tarballs/mediaelch.orig.tar.gz master
	#
	pushd ${FullSrcPath} > /dev/null
	bzr init &> /dev/null || echo "bzr already exists"
	bzr add . 1> /dev/null
	bzr commit -m "Commit for Debian packaging."
	bzr builddeb -- -us -uc
	popd > /dev/null
	msg_info "Debian package (.deb) created successfully."
}

build_windows() {
	msg_important "Building version $AppVersionStrFull for Windows using mxe..."
	
	mkdir -p "${ReleasePath}"
	pushd "${ReleasePath}" > /dev/null

	MXEDIR="/usr/lib/mxe"
	MXETARGET="i686-w64-mingw32.static"

	# Simple check if mxe exists.
	if [ ! -d "${MXEDIR}" ]; then
		msg_error "mxe (${MXEDIR}) not found!"
		print_help_windows
		cd $FullExecPath
		exit 1
	fi

	if [ ! -d "${FullSrcPath}/MediaInfoDLL" ]; then
		msg_info "Downloading MediaInfoDLL headers"
		svn checkout https://github.com/MediaArea/MediaInfoLib/trunk/Source/MediaInfoDLL $FullSrcPath/MediaInfoDLL
	fi
	if [ ! -d "${FullSrcPath}/ZenLib" ]; then
		msg_info "Downloading ZenLib headers"
		svn checkout https://github.com/MediaArea/ZenLib/trunk/Source/ZenLib $FullSrcPath/ZenLib
	fi

	PATH=$PATH:/usr/lib/mxe/usr/bin

	msg_info "Exporting CC = ${MXETARGET}-gcc"
	CC="${MXETARGET}-gcc"
	msg_info "Exporting CXX = ${MXETARGET}-g++"
	CXX="${MXETARGET}-g++"

	$MXEDIR/usr/bin/${MXETARGET}-qmake-qt5 ${FullSrcPath}/MediaElch.pro MXE=1
	make -j${JOBS} 1> /dev/null;

	popd > /dev/null
}

deploy_windows_exe() {
	build_windows
	cp ${ReleasePath}/release/ḾediaElch.exe ${DeployPath}/
}

build_mac() {
	msg_important "Building version $AppVersionStrFull for macOS..."
	
	mkdir -p "${ReleasePath}"
	pushd "${ReleasePath}" > /dev/null

	qmake ${FullSrcPath}/MediaElch.pro -spec macx-clang
	make -j${JOBS} 1> /dev/null

	popd > /dev/null
}

if [ "$BuildTarget" == "linux" ]; then
	if [ "$TargetType" == "AppImage" ]; then
		deploy_appimage

	elif [ "$TargetType" == "deb" ]; then
		deploy_deb_package

	else
		msg_error "\n Invalid TargetType for linux!"
		print_help
		cd $FullExecPath
		exit 1
	fi

elif [ "$BuildTarget" == "win" ]; then
	TargetType="exe"
	deploy_windows_exe

elif [ "$BuildTarget" == "mac" ]; then
	TargetType="dmg"
	build_mac
else
	msg_error "\n Invalid target!"
	print_help
	cd $FullExecPath
	exit 1
fi

msg_success "MediaElch release for »${BuildTarget} | ${TargetType}« complete."
msg_info "Build files can be found at ${ReleasePath}"
msg_info "Files can be found at ${DeployPath}"
cd ${FullExecPath}
