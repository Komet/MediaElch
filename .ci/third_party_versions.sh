#!/usr/bin/env sh

# Versions and hashes for MediaElch's dependencies.

###########################################################
# macOS

export MAC_MEDIAINFO_VERSION="21.09"
export MAC_MEDIAINFO_URL="https://mediaarea.net/download/binary/libmediainfo0/${MAC_MEDIAINFO_VERSION}/MediaInfo_DLL_${MAC_MEDIAINFO_VERSION}_Mac_x86_64+arm64.tar.bz2"
export MAC_MEDIAINFO_SHA512="763acedfdf6b62c52199d70c5a1332ed93bba70553946493925bd164e0f1c64b4f8cd6c2645230da0c334e6f77ee55dd72f18037c8d89d9f42683e1586365c3a"

export MAC_FFMPEG_VERSION="5.0"
export MAC_FFMPEG_URL="https://evermeet.cx/ffmpeg/ffmpeg-${MAC_FFMPEG_VERSION}.7z"
export MAC_FFMPEG_SHA512="ddac583241bb75ae0cb9b48be9dfff399f3d80679e3fc096b911a3e784b2453e880ab9d84c762ffc78f8b5f318d970cc3d2c83078b75cc110f080a4263ac820c"

export MAC_CREATE_DMG_GIT_REPO="https://github.com/andreyvit/create-dmg.git"
export MAC_CREATE_DMG_GIT_HASH="11a1b1c93cc1af6bcafc36dfad80ff41e789e65f"

###########################################################
# Windows

export WIN_MEDIAINFO_VERSION="21.09"
export WIN_MEDIAINFO_URL="https://mediaarea.net/download/binary/libmediainfo0/${WIN_MEDIAINFO_VERSION}/MediaInfo_DLL_${WIN_MEDIAINFO_VERSION}_Windows_x64_WithoutInstaller.7z"
export WIN_MEDIAINFO_SHA512="0cd336d9f5788ca879ddb70235a65efabefd44db996627e25766d3887e248f3a183ec866c91adc891ce0f2207d19cbdaf45adaa4f69d2d8779b9ac21903195e0"

# From https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
# Mirror at https://files.ameyering.de/binaries/Windows/ffmpeg/
export WIN_FFMPEG_VERSION="5.0"
export WIN_FFMPEG_ARCHIVE_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build.zip"
export WIN_FFMPEG_FOLDER_NAME="ffmpeg-${WIN_FFMPEG_VERSION}-essentials_build"
export WIN_FFMPEG_URL="https://files.ameyering.de/binaries/Windows/ffmpeg/${WIN_FFMPEG_ARCHIVE_NAME}"
export WIN_FFMPEG_SHA512="5d85285aa5b3772b1635e837bd7a7366440b8f53f81c70a299c1f7dec92015d73f02022344aab1984e033e0b1e4a238e27c294d9873b41f02034f2996e1253ad"

###########################################################
# Linux

# From https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
# Mirror at https://files.ameyering.de/binaries/Linux/ffmpeg/
export LINUX_FFMPEG_VERSION="5.0"
export LINUX_FFMPEG_ARCHIVE_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static.tar.xz"
export LINUX_FFMPEG_FOLDER_NAME="ffmpeg-${LINUX_FFMPEG_VERSION}-amd64-static"
export LINUX_FFMPEG_URL="https://files.ameyering.de/binaries/Linux/ffmpeg/${LINUX_FFMPEG_ARCHIVE_NAME}"
export LINUX_FFMPEG_SHA512="0d3f32727dfe3a0ed108f894c0bee770829aafca72fc5cfcc141f5f04ace75a959cd8011c74334e432b3df6d613719ba5bfe99eec59e3ad8eb60a2c1cc383ed6"
