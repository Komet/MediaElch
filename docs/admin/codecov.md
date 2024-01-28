# Codecov

__State__: last updated 2024-01-28

## What is Codecov?

Codecov is a tool to visualize code coverage of projects.  It's free for
open source projects.

- Website: <https://about.codecov.io/>
- MediaElch Project: <https://app.codecov.io/gh/Komet/MediaElch>


## How to publish MediaElch's coverage?

You need the latest `lcov` version first:

```sh
git clone https://github.com/linux-test-project/lcov.git
cd lcov
sudo make install 
```

<!-- Note: Keep in sync with ../contributing/testing.md -->

Then build MediaElch using GCC 12 with coverage enabled
and run target `coverage` which runs our integration and unit tests.
See [`testing.md`](../contributing/testing.md) for a guide and troubleshooting notes.

Then use the [Codecov Uploader](https://docs.codecov.com/docs/codecov-uploader):

```sh
# In MediaElch's source directory
cmake --preset debug-gcc
cmake --build --preset debug-gcc
cmake --build --preset debug-gcc --target coverage

cd build/debug-gcc

curl -Os https://uploader.codecov.io/latest/linux/codecov
chmod +x codecov

export CODECOV_TOKEN="<YOUR TOKEN>"
unset NODE_OPTIONS # See Codecov documentation; workaround as of 2023-05-11
./codecov --token "${CODECOV_TOKEN}" --branch master --file=coverage-merged.info
```


## Who has write-access to CodeCov?

The current CodeCov-maintainer is GitHub user
[`bugwelle`](https://github.com/bugwelle/).
