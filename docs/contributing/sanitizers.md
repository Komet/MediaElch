# Sanitizers

MediaElch uses GCC's and Clang's AddressSanitizer which allows us to find
memory corruption issues like segmentation faults before it reaches our
users.

Read more about the AddressSanitizer at:
<https://github.com/google/sanitizers/wiki/AddressSanitizer>

To build MediaElch with the AddressSanitizer enabled, you can use either
CMake or QMake.

Note that MediaElch has a few memory leaks due to how filters are handled.
Furthermore there are many false positives due to Qt.
I suggest to disable leak detection altogether for now.

## CMake

```sh
export ASAN_OPTIONS=detect_leaks=0
cmake .. -DCMAKE_BUILD_TYPE=Debug -DSANITIZE_ADDRESS=ON -GNinja
ninja
```

## QMake

```sh
export ASAN_OPTIONS=detect_leaks=0
mkdir build && cd build
qmake .. CONFIG+=sanitize
```
