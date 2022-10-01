# MediaElch Testing

Table of contents:

 - [Test types and folder structure](#test-types-and-folder-structure)
 - [Compile and run Tests](#compile-and-run-tests)
 - [Code Coverage](#code-coverage)
 - [Other checks](#other-checks)


## Test types and folder structure

MediaElch distinguishes between following test types, each of which has its
own subdirectory:

 - `unit`: Unit tests. Very fast to execute (<1s). No dependencies like reference files.
 - `scrapers`: Online scrapers tests. Requires an internet connection and
   can take two minutes or more to complete. 
 - `integration`: Integration tests which test all of MediaElch as one unit.
    Also contains unit-test-like tests for media_centers (Kodi NFO Tests).

`mocks` and `helpers` contain further C++ files that are helpful when writing tests.


## Compile and run Tests

As our test framework we use [Catch2](https://github.com/catchorg/Catch2).
With Catch2 you can create easy to write and easy to read tests.

In combination with CMake it's easy to test MediaElch:

```sh
# Run CMake with debug info enabled.
# We use Ninja as our build system instead of make.
cmake -S . -B build -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -GNinja

# You can also use the CMake preset `debug` from MediaElch's root,
# however, that also uses other tools such as USAN, Mold linker, etc.
# cmake --preset debug
# I recommend to use the AddressSanitizer preset:
# cmake --preset asan

# Build all of MediaElch including test executables:
cd build
ninja
# Execute all tests using CMake Test
ninja test
# Execute all unit tests
ninja unit_test
# Execute all integration tests (resource paths are automatically set)
ninja integration_test
# Execute all integration tests; diffs against reference files are written to disk
# (for more see below)
export MEDIAELCH_UPDATE_REF_FILES=1 && ninja integration_test
# Execute all scraper tests
ninja scraper_test
# Execute all scraper tests; diffs against reference files are written to disk
# (for more see below)
export MEDIAELCH_UPDATE_REF_FILES=1 && ninja scraper_test
# Don't rely on CMake custom target:
./test/scrapers/mediaelch_test_scrapers \
  --use-colour yes \
  --resource-dir ../test/resources \
  --temp-dir test/resources '[filter]'
```

If you don't like CMake's test output, you can also run MediaElch's tests on your own.
There are three test executables in the build folder after compiling everything:

 - `build/test/unit/mediaelch_unit`
 - `build/test/scrapers/mediaelch_test_scrapers`
 - `build/test/scrapers/mediaelch_test_integration`

All use Catch2 and therefore have the same command line interface, e.g.

```sh
# Test options
./mediaelch_unit -h            # List Catch2 help
./mediaelch_unit -t            # List all tags
./mediaelch_unit -d yes        # Run *all* tests and print duration
./mediaelch_unit "[load_data]" # Run scraping tests (online test)
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

```sh
# Use GCC
export CC=gcc-12
export CXX=g++-12
# Run CMake with coverage enabled and debug infos.
# We use Ninja as our build system instead of make.
cmake -S . -B build/coverage \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_TESTS=ON \
  -DENABLE_COVERAGE=ON \
  -GNinja
# build all of MediaElch including test executables
cd build/coverage
ninja
# runs the target `test` (see above) and the creates an HTML
# coverage report in build/coverage`
ninja coverage
```


## Other checks

MediaElch uses Coverity for further security checks.
See [`coverity.md`](../admin/coverity.md).
