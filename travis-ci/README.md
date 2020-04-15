# Travis CI

## Running Tests
MediaElch requires a GUI for its tests.
We use xvfb (X Virtual Framebuffer) to simulate a GUI on Travis CI
(see https://docs.travis-ci.com/user/gui-and-headless-browsers/#using-xvfb-to-run-tests-that-require-a-gui).

## Windows Build

The build scripts for Windows all rely on this docker image:
https://hub.docker.com/repository/docker/archer96/mediaelch-mxe-qt
