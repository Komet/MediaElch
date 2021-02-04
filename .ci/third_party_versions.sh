#!/usr/bin/env sh

# Versions and hashes for MediaElch's dependencies.

export MAC_MEDIAINFO_VERSION="20.09"
export MAC_FFMPEG_VERSION="4.3.1"
# See https://github.com/andreyvit/create-dmg.git
export MAC_CREATE_DMG_GIT_HASH=fbe0f36c823adbcbdcc15f9d65c6354252ac8307

export WIN_MEDIAINFO_VERSION="20.03"
# From https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
export WIN_FFMPEG_VERSION="ffmpeg-4.3.2-2021-02-02-essentials_build"
export WIN_FFMPEG_SHA512="9a0c5b0cfff8cda42691e69c6e721ae3033373411b7bd3457d7f3f23b373711bac27ce9c59655923093c82578b0f779ae0147b4adf9091569ac9ed718a76cdc7  ffmpeg.zip"

export LINUX_FFMPEG_SHA512="4629887efe26e1473636639e19b42e040217089bbeefd4ca059234d64d9b8ae1e8f6b5b925eeda012ea0665af0fcbb9f68f667a34aa696e38a3644c2bc3fbf16  ffmpeg.tar.xz" # 4.3.1
