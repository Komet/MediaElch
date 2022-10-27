#!/usr/bin/env sh

# Versions and hashes for MediaElch's dependencies.

###########################################################
# macOS

# From https://mediaarea.net/download/binary/libmediainfo0/${VERSION}/MediaInfo_DLL_${VERSION}_Mac_x86_64+arm64.tar.bz2
# Mirror at https://files.ameyering.de/binaries/macOS/mediainfo/
export MAC_MEDIAINFO_VERSION="22.09"
export MAC_MEDIAINFO_URL="https://files.ameyering.de/binaries/macOS/mediainfo/MediaInfo_DLL_${MAC_MEDIAINFO_VERSION}_Mac_x86_64+arm64.tar.bz2"
export MAC_MEDIAINFO_SHA512="645bdc16f469d6bbc2e476366dd670b9effb29e1ac83141c8dd1c7e1b5e50b66fb0f5eb799f3a9c244f049522a9f69ad4ee2315baf456acf8e53183b7c7e98ef"

# From https://evermeet.cx/ffmpeg/
# Mirror at https://files.ameyering.de/binaries/macOS/ffmpeg/
export MAC_FFMPEG_VERSION="5.1.2"
export MAC_FFMPEG_URL="https://files.ameyering.de/binaries/macOS/ffmpeg/ffmpeg-${MAC_FFMPEG_VERSION}.7z"
export MAC_FFMPEG_SHA512="e9dc121daae28e2176d80cc2a3b632aa6413b8532677a3093220fca6aba8420250e318e3ebd1fd44c8e77df235c57fa51264bf38666ee5b6165dcc2428e82883"

export MAC_CREATE_DMG_GIT_REPO="https://github.com/andreyvit/create-dmg.git"
export MAC_CREATE_DMG_GIT_HASH="412e99352bacef0f05f9abe6cc4348a627b7ac56" # 2022-07-20

###########################################################
# Windows

export WIN_MEDIAINFO_VERSION="21.09"
export WIN_MEDIAINFO_URL="https://mediaarea.net/download/binary/libmediainfo0/${WIN_MEDIAINFO_VERSION}/MediaInfo_DLL_${WIN_MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z"
export WIN_MEDIAINFO_SHA512="0cd336d9f5788ca879ddb70235a65efabefd44db996627e25766d3887e248f3a183ec866c91adc891ce0f2207d19cbdaf45adaa4f69d2d8779b9ac21903195e0"

# From https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
# Mirror at https://files.ameyering.de/binaries/Windows/ffmpeg/
export WIN_FFMPEG_VERSION="5.1.2"
export WIN_FFMPEG_ARCHIVE_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build.zip"
export WIN_FFMPEG_FOLDER_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build"
export WIN_FFMPEG_URL="https://files.ameyering.de/binaries/Windows/ffmpeg/${WIN_FFMPEG_ARCHIVE_NAME}"
export WIN_FFMPEG_SHA512="5d85285aa5b3772b1635e837bd7a7366440b8f53f81c70a299c1f7dec92015d73f02022344aab1984e033e0b1e4a238e27c294d9873b41f02034f2996e1253ad"

###########################################################
# Linux

# From https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
# Mirror at https://files.ameyering.de/binaries/Linux/ffmpeg/
export LINUX_FFMPEG_VERSION="5.1.1"
export LINUX_FFMPEG_ARCHIVE_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static.tar.xz"
export LINUX_FFMPEG_FOLDER_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static"
export LINUX_FFMPEG_URL="https://files.ameyering.de/binaries/Linux/ffmpeg/${LINUX_FFMPEG_ARCHIVE_NAME}"
export LINUX_FFMPEG_SHA512="0d3f32727dfe3a0ed108f894c0bee770829aafca72fc5cfcc141f5f04ace75a959cd8011c74334e432b3df6d613719ba5bfe99eec59e3ad8eb60a2c1cc383ed6"
