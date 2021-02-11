# MediaElch Docker builds

This directory contains Dockerfiles for the purpose of building
and testing MediaElch on different operating systems.

They are only partially integrated into our CI.

## Usage

```sh
./docker-build-all            # Build MediaElch with all Docker images
./docker-build-dist <distro>  # Build MediaElch with specific Docker image
```

Note that these scripts will create their build folders in `MediaElch/build/build-distro`.
You may wish to delete the build directory prior to executing above scripts.

## Linux CI Docker Image

`Dockerfile.ci.linux` is meant for our Jenkins pipeline.  It is rather large
and contains a lot of tools for testing purposes.

See `publish_images_to_dockerhub.sh` for more details.

## Windows MXE

The Windows MXE docker image is handled differently. It takes more than
an hour on my 4 core machine to build the image. Because of that I pushed it
to <https://hub.docker.com/repository/docker/mediaelch/mediaelch-ci-win>

```sh
cd ./.ci/docker
# Build Image
docker build -t mediaelch/mediaelch-ci-win:latest -f Dockerfile.ci.windows .
# Push
docker push mediaelch/mediaelch-ci-win:latest
```
