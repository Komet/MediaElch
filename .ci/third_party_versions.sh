#!/usr/bin/env sh

# Versions and checksums for MediaElch's dependencies.

###########################################################
# macOS

export MAC_QT_6_VERSION="6.7.0"
export MAC_QT_5_VERSION="5.15.2"
# From https://mediaarea.net/download/binary/libmediainfo0/${MAC_MEDIAINFO_VERSION}/MediaInfo_DLL_${MAC_MEDIAINFO_VERSION}_Mac_x86_64+arm64.tar.bz2
# Mirror at https://files.ameyering.de/binaries/macOS/mediainfo/
# Version is also used as Git tag for downloading header files.
export MAC_MEDIAINFO_VERSION="24.03"
export MAC_MEDIAINFO_URL="https://files.ameyering.de/binaries/macOS/mediainfo/MediaInfo_DLL_${MAC_MEDIAINFO_VERSION}_Mac_x86_64+arm64.tar.bz2"
export MAC_MEDIAINFO_SHA512="9a940b790767e1b4e9a52ebffb9f62897d053bc6068cea47a94eed59fd9d6640806f065d778519b90ebb96cd808b880c8749d42c7355c217b1e91150188f2897"

# From https://evermeet.cx/ffmpeg/
# Mirror at https://files.ameyering.de/binaries/macOS/ffmpeg/
export MAC_FFMPEG_VERSION="6.1.1"
export MAC_FFMPEG_URL="https://files.ameyering.de/binaries/macOS/ffmpeg/ffmpeg-${MAC_FFMPEG_VERSION}.7z"
export MAC_FFMPEG_SHA512="de63b017c0f2488133720a26f3bd42693cec2ed205286ea548b995deda44ea7f70251c3b5db21ab6fb16d15c9694adc259753a45a6254e8868c6dcc7a423bdb5"

export MAC_CREATE_DMG_GIT_REPO="https://github.com/andreyvit/create-dmg.git"
export MAC_CREATE_DMG_GIT_HASH="f475bcfdd236803b0e5956bdbc3eaab2b09f4e96" # 2024-04-07

###########################################################
# Windows

# From https://mediaarea.net/download/binary/libmediainfo0/${WIN_MEDIAINFO_VERSION}/MediaInfo_DLL_${WIN_MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z
# Mirror at https://files.ameyering.de/binaries/Windows/mediainfo/
export WIN_MEDIAINFO_VERSION="24.03"
export WIN_MEDIAINFO_URL="https://files.ameyering.de/binaries/Windows/mediainfo/MediaInfo_DLL_${WIN_MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z"
export WIN_MEDIAINFO_SHA512="497521c3fc6407fdc4e74b01607c24503b847350930fd366b074026cb96bd515842f87261805e939d8f9b3ded7717b65c429695f40a5a398009f97fccc80d38e"

# From https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
# Mirror at https://files.ameyering.de/binaries/Windows/ffmpeg/
export WIN_FFMPEG_VERSION="7.0"
export WIN_FFMPEG_ARCHIVE_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build.zip"
export WIN_FFMPEG_FOLDER_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build"
export WIN_FFMPEG_URL="https://files.ameyering.de/binaries/Windows/ffmpeg/${WIN_FFMPEG_ARCHIVE_NAME}"
export WIN_FFMPEG_SHA512="d458d8052b2810983b3bb963846bd30c4682776b422fcd7856d54b0874c040f932352361fe5ad52cd1e8ef919b2db5542ab20f2d4d88d30d9fd8d59860f28142"

###########################################################
# Linux

# From https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
# Mirror at https://files.ameyering.de/binaries/Linux/ffmpeg/
export LINUX_FFMPEG_VERSION="6.1"
export LINUX_FFMPEG_ARCHIVE_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static.tar.xz"
export LINUX_FFMPEG_FOLDER_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static"
export LINUX_FFMPEG_URL="https://files.ameyering.de/binaries/Linux/ffmpeg/${LINUX_FFMPEG_ARCHIVE_NAME}"
export LINUX_FFMPEG_SHA512="01852a016d54a8f866794ee767fba860d0f407a10afb5f92cd920cdc2cc235ad02c7b1e5b70728ca89ba8165e8ae4c25b51de0e021a3fa35197fa07890736ea7"
