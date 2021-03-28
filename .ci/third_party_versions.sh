#!/usr/bin/env sh

# Versions and hashes for MediaElch's dependencies.

export MAC_MEDIAINFO_VERSION="20.09"
export MAC_FFMPEG_VERSION="4.3.1"
# See https://github.com/andreyvit/create-dmg.git
export MAC_CREATE_DMG_GIT_HASH=fbe0f36c823adbcbdcc15f9d65c6354252ac8307

export WIN_MEDIAINFO_VERSION="20.03"
# From https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
export WIN_FFMPEG_VERSION="ffmpeg-4.3.2-2021-02-27-essentials_build"
export WIN_FFMPEG_SHA512="2e0a8ebf671b2ed3c13e47f7b3cdf02f45ad95c85b768531572c024b29a60a01a1947ecfdc769033fd704329af627b63df631827c4d83a1c4effc75397af73eb  ffmpeg.zip"

# From https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
export LINUX_FFMPEG_SHA512="d03d31378c3225d39a90ff43f612041c0e90ed4e8196b7074f7908fbb4d1b820efdee4c08bdcd334c1ea626c6c25a3dd19183ebcfd7535c727ff67d1cccb8b65  ffmpeg.tar.xz" # 4.3.1
