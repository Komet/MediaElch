#!/usr/bin/env sh

# Versions and checksums for MediaElch's dependencies.

###########################################################
# macOS

export MAC_QT_6_VERSION="6.8.0"
export MAC_QT_5_VERSION="5.15.2"
# From https://mediaarea.net/download/binary/libmediainfo0/${MAC_MEDIAINFO_VERSION}/MediaInfo_DLL_${MAC_MEDIAINFO_VERSION}_Mac_x86_64+arm64.tar.bz2
# Mirror at https://files.ameyering.de/binaries/macOS/mediainfo/
# Version is also used as Git tag for downloading header files.
export MAC_MEDIAINFO_VERSION="24.06"
export MAC_MEDIAINFO_URL="https://files.ameyering.de/binaries/macOS/mediainfo/MediaInfo_DLL_${MAC_MEDIAINFO_VERSION}_Mac_x86_64+arm64.tar.bz2"
export MAC_MEDIAINFO_SHA512="d4522d0e7137bd0b57128a0ad260e31c0739983a1e0711adfb0a849face4391b607b964bec618bd6c29518b13fb956de4e101ae553f9cc92570df331bda7070f"

# From https://evermeet.cx/ffmpeg/
# Mirror at https://files.ameyering.de/binaries/macOS/ffmpeg/
export MAC_FFMPEG_VERSION="7.0.2"
export MAC_FFMPEG_URL="https://files.ameyering.de/binaries/macOS/ffmpeg/ffmpeg-${MAC_FFMPEG_VERSION}.7z"
export MAC_FFMPEG_SHA512="d7ba14c16531e911cbfb303d1cf2bd34d87dbfe7ce99fa9f98af8951b4506abb731b904685430b227fee46c16d694d489836660218d73db4805e4bea6e3522d3"

export MAC_CREATE_DMG_GIT_REPO="https://github.com/andreyvit/create-dmg.git"
export MAC_CREATE_DMG_GIT_HASH="32121505917f6eb83c9a5144a80e4f5c95557fef" # 2024-09-22

###########################################################
# Windows

# From https://mediaarea.net/download/binary/libmediainfo0/${WIN_MEDIAINFO_VERSION}/MediaInfo_DLL_${WIN_MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z
# Mirror at https://files.ameyering.de/binaries/Windows/mediainfo/
export WIN_MEDIAINFO_VERSION="24.06"
export WIN_MEDIAINFO_URL="https://files.ameyering.de/binaries/Windows/mediainfo/MediaInfo_DLL_${WIN_MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z"
export WIN_MEDIAINFO_SHA512="7b71110ad0ca643e52c3390ba9bd7615c4c077bba84b3a68c9b05b28e90bce8670c0905863cb7dca359118cbb64dca324bac97262925ae555eeeac0e722bcd5a"

# From https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
# Mirror at https://files.ameyering.de/binaries/Windows/ffmpeg/
export WIN_FFMPEG_VERSION="7.0.2"
export WIN_FFMPEG_ARCHIVE_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build.zip"
export WIN_FFMPEG_FOLDER_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build"
export WIN_FFMPEG_URL="https://files.ameyering.de/binaries/Windows/ffmpeg/${WIN_FFMPEG_ARCHIVE_NAME}"
export WIN_FFMPEG_SHA512="05842a10f106826375859a91431e464cab334d153a16e350280e9e62b9e2f4c44e3cfbe297575cb197c8308b634cad6ca75c45f1bc209c6f6c0b6d2f18b83b09"

###########################################################
# Linux

# From https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
# Mirror at https://files.ameyering.de/binaries/Linux/ffmpeg/
export LINUX_FFMPEG_VERSION="7.0.2"
export LINUX_FFMPEG_ARCHIVE_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static.tar.xz"
export LINUX_FFMPEG_FOLDER_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static"
export LINUX_FFMPEG_URL="https://files.ameyering.de/binaries/Linux/ffmpeg/${LINUX_FFMPEG_ARCHIVE_NAME}"
export LINUX_FFMPEG_SHA512="e80880362208de7437f0eb98d25ec6676df122a04613a818bb14101c3e1ccf91feab06246d40359b887b147af65d80eef5dba99964cac469bcb560f1a063d737"
