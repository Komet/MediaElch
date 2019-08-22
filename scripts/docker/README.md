# Dockerfile for MediaElch development

`./scripts/docker` contains a Dockerfile that can be used for MediaElch
development. It has recent versions of compilers, Qt, CMake and so on.

## Build Docker Image

```sh
cd scripts/docker
docker build -t "mediaelch-build:dev" .
```

## Using the Docker Image

```sh
docker run --rm  -it -v $(pwd):/opt/mediaelch mediaelch-build:dev /bin/bash
```

## Notes on running tests

Because our tests require a GUI (even unit tests at the moment) we need a
display. You can accomplish this by using `xvfb`. To run our coverage tests
you can then use:

```sh
cd path/to/build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -GNinja
xvfb-run ninja coverage
# [ Output... ]
#   lines......: 7.2% (2606 of 36318 lines)
#   functions..: 11.4% (503 of 4418 functions)
```
