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
./.ci/macOS/build_macOS_release.sh
./.ci/macOS/package_macOS.sh
# linux
./.ci/linux/build_linux_release_in_docker.sh
./.ci/linux/package_linux_appimage_in_docker.sh
# win
./.ci/win/build_windows_release_release_in_docker.sh
./.ci/win/package_windows_in_docker.sh
# Gather Packages
cd "${ME_ROOT}"
mv build/linux/MediaElch*.AppImage .
mv build/macOS/MediaElch*.dmg .
mv build/win/MediaElch*.zip .
```

## TravisCI (to be removed)

### Running Tests

MediaElch requires a GUI for its tests.
We use xvfb (X Virtual Framebuffer) to simulate a GUI on Travis CI
(see <https://docs.travis-ci.com/user/gui-and-headless-browsers/#using-xvfb-to-run-tests-that-require-a-gui>).

## Windows Build

The build scripts for Windows all rely on this docker image:
<https://hub.docker.com/repository/docker/mediaelch/mediaelch-ci-win>
