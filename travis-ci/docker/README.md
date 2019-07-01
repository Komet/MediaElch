# MediaElch Docker builds

This directory contains Dockerfiles for the purpose of building
and testing MediaElch on different operating systems.

## Usage

```sh
./docker-build-all            # Build MediaElch with all Docker images
./docker-build-dist <distro>  # Build MediaElch with specific Docker image
```

Note that these scripts will create their build folders in `MediaElch/build/build-distro`.
You may wish to delete the build directory prior to executing above scripts.
