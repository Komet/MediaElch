#!/usr/bin/env sh

# Versions and checksums for MediaElch's dependencies.

###########################################################
# macOS

# From https://mediaarea.net/download/binary/libmediainfo0/${MAC_MEDIAINFO_VERSION}/MediaInfo_DLL_${MAC_MEDIAINFO_VERSION}_Mac_x86_64+arm64.tar.bz2
# Mirror at https://files.ameyering.de/binaries/macOS/mediainfo/
export MAC_MEDIAINFO_VERSION="23.07"
export MAC_MEDIAINFO_URL="https://files.ameyering.de/binaries/macOS/mediainfo/MediaInfo_DLL_${MAC_MEDIAINFO_VERSION}_Mac_x86_64+arm64.tar.bz2"
export MAC_MEDIAINFO_SHA512="d5ce0996d6ef7b5fc9fcbb4cb4a8bbd1957858a65e9773f4f520a156ed8c25b33e73c164baffafa67ec2f72dc5807644eb83dc27463792be0d63e8b539fc456e"

# From https://evermeet.cx/ffmpeg/
# Mirror at https://files.ameyering.de/binaries/macOS/ffmpeg/
export MAC_FFMPEG_VERSION="6.0"
export MAC_FFMPEG_URL="https://files.ameyering.de/binaries/macOS/ffmpeg/ffmpeg-${MAC_FFMPEG_VERSION}.7z"
export MAC_FFMPEG_SHA512="b60004f2974dd3c4da51608ee953628b8b9517d481ab1153cb5e1e2295609f98deab3508246774317a8578dc284861c4a07234cb703f39e736c4fd4ad2ef3ed4"

export MAC_CREATE_DMG_GIT_REPO="https://github.com/andreyvit/create-dmg.git"
export MAC_CREATE_DMG_GIT_HASH="c89d743919acb1a16259ed7b98059393978fb639" # 2023-03-25

###########################################################
# Windows

# From https://mediaarea.net/download/binary/libmediainfo0/${WIN_MEDIAINFO_VERSION}/MediaInfo_DLL_${WIN_MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z
# Mirror at https://files.ameyering.de/binaries/Windows/mediainfo/
export WIN_MEDIAINFO_VERSION="23.07"
export WIN_MEDIAINFO_URL="https://files.ameyering.de/binaries/Windows/mediainfo/MediaInfo_DLL_${WIN_MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z"
export WIN_MEDIAINFO_SHA512="c10152046b4b9d9b98765f268f6f71ee7df31c83ee0a86e0ca9533756926ae73b73685d825f08041283ea90edbca3b798b2744ed463cdee9f2cd2c8505c82327"

# From https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
# Mirror at https://files.ameyering.de/binaries/Windows/ffmpeg/
export WIN_FFMPEG_VERSION="6.0"
export WIN_FFMPEG_ARCHIVE_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build.zip"
export WIN_FFMPEG_FOLDER_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build"
export WIN_FFMPEG_URL="https://files.ameyering.de/binaries/Windows/ffmpeg/${WIN_FFMPEG_ARCHIVE_NAME}"
export WIN_FFMPEG_SHA512="1833605897033ed68826e55c5fa977e0f3086cd41fab75f5361b5c08f29e0f71ae85b0b499b31142e3eb17bd1885ea0c979f2e2fa64a9e01ce60c23c73527075"

###########################################################
# Linux

# From https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
# Mirror at https://files.ameyering.de/binaries/Linux/ffmpeg/
export LINUX_FFMPEG_VERSION="6.0"
export LINUX_FFMPEG_ARCHIVE_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static.tar.xz"
export LINUX_FFMPEG_FOLDER_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static"
export LINUX_FFMPEG_URL="https://files.ameyering.de/binaries/Linux/ffmpeg/${LINUX_FFMPEG_ARCHIVE_NAME}"
export LINUX_FFMPEG_SHA512="e211bc1fd2b0352d02961883756987ec7c1a0e4dfb47773148498adabce71732b13a029cea5a7c6cef5540bd07ec7c73d11a095593ca5f6d5a116ddf40891191"
