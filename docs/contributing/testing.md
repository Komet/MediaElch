# MediaElch Testing

Table of contents:

 - Test types and folder structure
 - How to test
 - Code Coverage
 - Other checks


## Test types and folder structure
MediaElch distinguishes between following tests, each of which has its
own subdirectory:

 - `unit`: Unit tests. Very fast to execute (<1s). No dependencies like reference files.
 - `scrapers`: Online scrapers tests. Requires an internet connection and
   can take two minutes to complete. 
 - `integration`: Integration tests which test all of MediaElch as one unit.
    Also contains unit-test-like tests for media_centers.

`mocks` and `helpers` contain further C++ files that are helpful when writing tests.


## How to test
As our testframework we use [Catch2](https://github.com/catchorg/Catch2).
With Catch2 you can create easy to write and easy to read tests.

In combination with CMake it's easy to test MediaElch:

```sh
# create test directory
mkdir build && cd $_
# run CMake with debug infos enabled
# we use Ninja as our build system instead of make
cmake .. -DCMAKE_BUILD_TYPE=Debug -GNinja
# build all of MediaElch including test executables
ninja
# Execute all tests
ninja test
# Execute all unit tests
ninja unit_test
# Execute all integration tests
ninja integration_test
# Execute all scraper tests
ninja scraper_test
```

If you don't like CMake's test output, you can also run MediaElch's tests on your own.
There are three test executables in the build folder after compiling everything:

 - `build/test/unit/mediaelch_unit`
 - `build/test/scrapers/mediaelch_test_scrapers`
 - `build/test/scrapers/mediaelch_test_integration`

All use CMake and therefore have the same command line interface, e.g.

```sh
# Test options
./mediaelch_unit -h            # List Catch2 help
./mediaelch_unit -t            # List all tags
./mediaelch_unit -d yes        # Run *all* tests and print duration
./mediaelch_unit "[load_data]" # Run scraping tests (online test)
```


## Code Coverage

A CMake target exists to create Mediaelch's coverage: `coverage`

```sh
# create test directory
mkdir build && cd $_
# run CMake with coverage enabled and debug infos
# we use Ninja as our build system instead of make
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -GNinja
# build all of MediaElch including test executables
ninja
# runs the target `test` (see above) and the creates an HTML
# coverage report in build/coverage`
ninja coverage
```


## Other checks
MediaElch uses Coverity for further security checks.
See [`coverity.md`](../admin/coverity.md).
