#!/usr/bin/env sh

# Versions and hashes for MediaElch's dependencies.

export MAC_MEDIAINFO_VERSION="20.09"
export MAC_FFMPEG_VERSION="4.3.1"
# See https://github.com/andreyvit/create-dmg.git
export MAC_CREATE_DMG_GIT_HASH=fbe0f36c823adbcbdcc15f9d65c6354252ac8307

export WIN_MEDIAINFO_VERSION="20.03"
# From https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
# Mirror at https://files.ameyering.de/binaries/Windows/ffmpeg/
export WIN_FFMPEG_VERSION="4.4.1"
export WIN_FFMPEG_SHA512="e3282867447b58b4f1712d344296fa20ee85801cbb99e790fce1163e9a744688a03391a9c8c3afb1d6744b2a7b32240669d414ec188ffd30391c1a686b577b68  ffmpeg.zip"

# From https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
export LINUX_FFMPEG_SHA512="2ca6fb3279d80871fb79c30ef8799f7c87ad59ae31a22400d63c9d43ef86a36013387cbcb417ddc00be61eca32103e196bbb6a5d39ae2ce096716145fb69f48d  ffmpeg.tar.xz" # 4.4
