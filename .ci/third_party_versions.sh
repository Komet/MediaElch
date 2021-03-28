#!/usr/bin/env sh

# Versions and hashes for MediaElch's dependencies.

export MAC_MEDIAINFO_VERSION="20.09"
export MAC_FFMPEG_VERSION="4.3.1"
# See https://github.com/andreyvit/create-dmg.git
export MAC_CREATE_DMG_GIT_HASH=fbe0f36c823adbcbdcc15f9d65c6354252ac8307

export WIN_MEDIAINFO_VERSION="20.03"
# From https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
export WIN_FFMPEG_VERSION="ffmpeg-4.3.2-2021-02-27-essentials_build"
export WIN_FFMPEG_SHA512="d03d31378c3225d39a90ff43f612041c0e90ed4e8196b7074f7908fbb4d1b820efdee4c08bdcd334c1ea626c6c25a3dd19183ebcfd7535c727ff67d1cccb8b65  ffmpeg.zip"

export LINUX_FFMPEG_SHA512="4629887efe26e1473636639e19b42e040217089bbeefd4ca059234d64d9b8ae1e8f6b5b925eeda012ea0665af0fcbb9f68f667a34aa696e38a3644c2bc3fbf16  ffmpeg.tar.xz" # 4.3.1
