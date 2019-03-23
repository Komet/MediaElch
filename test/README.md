# MediaElch Testing

MediaElch uses [Catch2](https://github.com/catchorg/Catch2) for testing.

## How to test

```sh
mkdir build && cd $_
cmake ..
make -j4

# Execute all tests
make test
# Execute all unit tests
make unit
# Execute all tests
make scraper_test

# Test options
./test_exe -h            # List Catch2 help
./test_exe -t            # List all tags
./test_exe -d yes        # Run *all* tests and print duration
./test_exe "[load_data]" # Run scraping tests (online test)
./test_exe "[search]"    # Run scraping search tests (online test)
```

## Code Coverage

A CMake target exists to create Mediaelch's coverage: `coverage`

```sh
mkdir build && cd $_
cmake ..
make -j4
make coverage
```

## CMake and testing

### CTest and Catch2

By using `cmake/Catch.cmake` we can tell CTest to automatically search for Catch tests.

```cmake
catch_discover_tests(mediaelch_test_scrapers)
```

### Coverage

If `ENABLE_COVERAGE` is set and `make coverage` is executed, a coverage 
report should be generated. We have to register tests to be executed first, though:

```cmake
generate_coverage_report(mediaelch_test_scrapers)
```

### Cotire specifics

Cotire tries to use a PCH for Catch2 which results in `CATCH_CONFIG_RUNNER` not
having any effect. To avoid this, we exclude the main executable from cotire.

```cmake
set_source_files_properties(main.cpp PROPERTIES COTIRE_EXCLUDED ON)
```
