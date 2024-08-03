# Sanitizers

__State__: last updated 2024-08-03

MediaElch uses GCC's and Clang's AddressSanitizer which allows us to find
memory corruption issues like segmentation faults before it reaches our
users.

Read more about the AddressSanitizer at:
<https://github.com/google/sanitizers/wiki/AddressSanitizer>

To build MediaElch with the AddressSanitizer enabled, you can use either
CMake or QMake.

Note that MediaElch has a few memory leaks due to how filters are handled.
Furthermore, there are many false positives due to Qt.
I suggest to disable leak detection altogether for now.

## CMake

```sh
export ASAN_OPTIONS=detect_leaks=0
cmake .. -DCMAKE_BUILD_TYPE=Debug -DSANITIZE_ADDRESS=ON -GNinja
ninja
```

We also have a CMake preset for address sanitizer:

```sh
export ASAN_OPTIONS=detect_leaks=0
cmake --preset asan
cmake --build --preset asan
cd build/asan
```

## QMake

```sh
export ASAN_OPTIONS=detect_leaks=0
mkdir build && cd build
qmake .. CONFIG+=sanitize
```

## Troubleshooting

### No filenames or line numbers shown

Ensure that `llvm-symbolizer` is installed.
If it is not in your `$PATH`, set `ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer`.

See <https://stackoverflow.com/a/24572545/1603627>
