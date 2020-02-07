# Dockerfile for MediaElch development

`./scripts/docker` contains a Dockerfile that can be used for MediaElch
development. It has recent versions of compilers, Qt, CMake and so on.

## Build Docker Image

```sh
docker build -t "mediaelch-build:dev" .
```

## Using the Docker Image

```sh
docker run --rm  -it -v $(pwd):/opt/mediaelch mediaelch-build:dev /bin/bash
```
