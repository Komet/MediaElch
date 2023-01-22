# CI

## Local Nightlies

This directory contains scripts to create Nightlies and release
packages for Linux, macOS and Windows.

As the current maintainer has a MacBook, this guide assumes that
the scripts are run on macOS.

All in one script:

```sh
cd MediaElch
ME_ROOT="$(pwd)"
rm -rf build

# macOS
./.ci/macOS/build_macOS_release_Qt6.sh
./.ci/macOS/package_macOS_Qt6.sh
./.ci/macOS/build_macOS_release_Qt5.sh
./.ci/macOS/package_macOS_Qt5.sh

# linux
./.ci/linux/build_linux_release_in_docker.sh
./.ci/linux/package_linux_appimage_in_docker.sh

# win
./.ci/win/build_windows_release_qt6_in_docker.sh
./.ci/win/package_windows_qt6_in_docker.sh
./.ci/win/build_windows_release_qt5_in_docker.sh
./.ci/win/package_windows_qt5_in_docker.sh

# Gather Packages
cd "${ME_ROOT}"
mv build/linux/MediaElch*.AppImage .
mv build/macOS/MediaElch*.dmg .
mv build/win/MediaElch*.zip .
```


## Note on Checksums

We test that the checksum of our dependencies are correct.
However, this won't be the case if you want to package an older version of MediaElch.
If that's the case, you can set an environment variable to ignore invalid checksums.

```sh
export MEDIAELCH_IGNORE_CHECKSUMS="yes"
# to check again:
unset MEDIAELCH_IGNORE_CHECKSUMS
```


## Windows Build

The build scripts for Windows all rely on this docker image:
<https://hub.docker.com/repository/docker/mediaelch/mediaelch-ci-win>
