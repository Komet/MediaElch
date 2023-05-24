# MediaElch Testing

__State__: last updated 2023-05-24

Table of contents:

 - [Test types and folder structure](#test-types-and-folder-structure)
 - [Compile and run Tests](#compile-and-run-tests)
 - [Code Coverage](#code-coverage)
 - [Other checks](#other-checks)


## Test types and folder structure

MediaElch distinguishes between following test types, each of which has its
own subdirectory:

 - `unit`: Unit tests. Very fast to execute (<1s). No dependencies like reference files.
 - `scrapers`: Online scraper tests.  Requires an internet connection and
   can take two minutes or more to complete. 
 - `integration`: Integration tests which test all of MediaElch as one unit.
    Also contains unit-test-like tests for media_centers (Kodi NFO Tests).

`mocks` and `helpers` contain further C++ files that are helpful when writing tests.

The folder `resources` contains reference files.  These are files that we
load and compare a function's output against.  They are used for integration and
scraper tests.


## Compile and run Tests

As our test framework we use [Catch2](https://github.com/catchorg/Catch2).
With Catch2 you can create easy to write and easy to read tests.

In combination with CMake it's easy to test MediaElch:

```sh
# Run CMake with debug info enabled.
# We use Ninja as our build system instead of make.
cmake -S . -B build -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -GNinja
# Build all of MediaElch including test executables:
cd build && ninja

# You can also use the CMake preset `debug` from MediaElch's root,
# however, that also uses other tools such as USAN, Mold linker, etc.
#
#   cmake --preset debug && cmake --build --preset debug
#
# I recommend to use the AddressSanitizer preset:
#
#   cmake --preset asan && cmake --build --preset asan

# Execute all unit tests
ninja unit_test

# Execute all integration tests (resource paths are set via CMake)
ninja integration_test
# …or via direct executable (resource paths are set via macros through CMake at build time)
./test/integration/mediaelch_test_integration

# Execute all scraper tests (resource paths are set via CMake)
ninja scraper_test
# …or via direct executable (resource paths are set via macros through CMake at build time)
./test/scrapers/mediaelch_test_scrapers 

# The CMake targets are convenience for the long variants.
# Via compile definitions, the source directory path is part of the test executable,
# so explicit options are not necessary.
./test/scrapers/mediaelch_test_scrapers \
  --use-colour yes \
  --resource-dir ../test/resources \
  --temp-dir test/resources '[filter]'
```

There are three test executables in the build folder after compiling everything:

 - `build/test/unit/mediaelch_unit_test`
 - `build/test/scrapers/mediaelch_test_scrapers`
 - `build/test/scrapers/mediaelch_test_integration`

All use Catch2 and therefore have the same command line interface, e.g.

```sh
# Test options
./mediaelch_unit_test -h            # List Catch2 help
./mediaelch_unit_test -t            # List all tags
./mediaelch_unit_test -d yes        # Run *all* tests and print duration
./mediaelch_unit_test "[load_data]" # Run scraping tests (online test)
```

Since integration tests and scraper tests check their output against reference
files (found in `test/resources`), you may need to update them if you change
MediaElch's code.  
To ease that, you can set the environment variable `MEDIAELCH_UPDATE_REF_FILES`.

```sh
export MEDIAELCH_UPDATE_REF_FILES=1
ninja integration_test
ninja scraper_test
```


## Code Coverage

A CMake target exists to create Mediaelch's coverage: `coverage`

<!-- Note: Keep in sync with ../admin/coverity.md -->
```sh
# Use GCC; versions must match (also for gcov)
export CC=gcc
export CXX=g++
# Run CMake with coverage enabled and debug infos.
# We use Ninja as our build system instead of make.
export ASAN_OPTIONS=detect_leaks=0
cmake -S . -B build/coverage \
  -DDISABLE_UPDATER=ON \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSANITIZE_ADDRESS=ON \
  -DENABLE_TESTS=ON \
  -DENABLE_COVERAGE=ON \
  -DMEDIAELCH_FORCE_QT5=ON \
  -DMEDIAELCH_FORCE_QT6=OFF \
  -GNinja
# build all of MediaElch including test executables
cd build/coverage
ninja
# runs the target `test` (see above) and the creates an HTML
# coverage report in build/coverage`
ninja coverage
```

You can also use the CMake preset `ci`.

__Troubleshooting__:

 - _`.gcno:version '404*', prefer '407*'`_  
   See [StackOverflow](https://stackoverflow.com/questions/12454175/gcov-out-of-memory-mismatched-version).
   Likely a version mismatch, e.g. GCC12, GCOV11.
 - _No GCOV data generated_  
   If you have multiple builds next to each other, e.g. `build/asan` and `build/ci`,
   check that LCOV did not accidentally use `build/asan` when only `build/ci`
   has coverage enabled.
 - _Mismatch error_  
   See <https://github.com/linux-test-project/lcov/issues/209>.  In the past, adapting the coding slightly helped.

## Other checks

MediaElch uses Coverity for further security checks.  It's automated via Jenkins.
See [`coverity.md`](../admin/coverity.md).
