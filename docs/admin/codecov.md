# Codecov

## What is Codecov?

Website: https://app.codecov.io/gh/Komet/MediaElch

## How to publish MediaElch's coverage?
You need the latest `lcov` version first:

```sh
git clone https://github.com/linux-test-project/lcov.git
cd lcov
sudo make install 
```

Then build MediaElch using GCC 9 (newer versions may not be supported by lcov).

```sh
git clone git@github.com:bugwelle/MediaElch.git
cd MediaElch
export CC=gcc-9
export CXX=g++-9
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_COVERAGE=ON \
    -DDISABLE_UPDATER=ON \
    -DENABLE_TESTS=ON \
    -GNinja
cmake --build build
cmake --build build --target coverage
```

Then use the [Codecov Uploader](https://docs.codecov.com/docs/codecov-uploader):

```sh
cd build
curl -Os https://uploader.codecov.io/latest/linux/codecov

chmod +x codecov
export CODECOV_TOKEN="<YOUR TOKEN>"
./codecov -t ${CODECOV_TOKEN}
```


## Who has write-access to CodeCov?
The current CodeCov-maintainer is GitHub user
[`bugwelle`](https://github.com/bugwelle/).
