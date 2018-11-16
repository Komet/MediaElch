# cmake-format: off
# Findlcov results:
# lcov_FOUND
# lcov_EXECUTABLE

include(FindPackageHandleStandardArgs)

# find gcov as it is used by lcov
find_package(gcov)

# work around CMP0053, see http://public.kitware.com/pipermail/cmake/2014-November/059117.html
set(PROGRAMFILES_x86_ENV "PROGRAMFILES(x86)")

find_program(lcov_EXECUTABLE
    NAMES
        lcov
    PATHS
        "${LCOV_DIR}"
        "$ENV{LCOV_DIR}"
        "$ENV{PROGRAMFILES}/lcov"
        "$ENV{${PROGRAMFILES_x86_ENV}}/lcov"
)

find_program(genhtml_EXECUTABLE
    NAMES
        genhtml
    PATHS
        "${LCOV_DIR}"
        "$ENV{LCOV_DIR}"
        "$ENV{PROGRAMFILES}/lcov"
        "$ENV{${PROGRAMFILES_x86_ENV}}/lcov"
)

find_package_handle_standard_args(lcov
    FOUND_VAR
        lcov_FOUND
    REQUIRED_VARS
        lcov_EXECUTABLE
        genhtml_EXECUTABLE
)

mark_as_advanced(
    lcov_EXECUTABLE
    genhtml_EXECUTABLE
)
# cmake-format: on
