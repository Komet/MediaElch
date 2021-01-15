# MediaElch Docker builds

This directory contains Dockerfiles for the purpose of building
and testing MediaElch on different operating systems.

They are not yet integrated into our CI.

They are not meant for release-binaries.

## Usage

```sh
./docker-build-all            # Build MediaElch with all Docker images
./docker-build-dist <distro>  # Build MediaElch with specific Docker image
```

Note that these scripts will create their build folders in `MediaElch/build/build-distro`.
You may wish to delete the build directory prior to executing above scripts.


## Windows MXE

The Windows MXE docker image is handled differently. It takes more than
an hour on my 4 core machine to build the image. Because of that I pushed it
to https://hub.docker.com/repository/docker/archer96/mediaelch-mxe-qt

```sh
cd ./.ci/docker
# Build Image
docker build -t archer96/mediaelch-mxe-qt:latest -f Dockerfile.ci.windows .
# Push
docker push archer96/mediaelch-mxe-qt:latest
```
