# cmake-format: off
# Findgcov results:
# gcov_FOUND
# gcov_EXECUTABLE

include(FindPackageHandleStandardArgs)

# work around CMP0053, see http://public.kitware.com/pipermail/cmake/2014-November/059117.html
set(PROGRAMFILES_x86_ENV "PROGRAMFILES(x86)")

find_program(gcov_EXECUTABLE
    NAMES
        gcov
    PATHS
        "${GCOV_DIR}"
        "$ENV{GCOV_DIR}"
        "$ENV{PROGRAMFILES}/gcov"
        "$ENV{${PROGRAMFILES_x86_ENV}}/gcov"
)

find_package_handle_standard_args(gcov
    FOUND_VAR
        gcov_FOUND
    REQUIRED_VARS
        gcov_EXECUTABLE
)

mark_as_advanced(gcov_EXECUTABLE)
# cmake-format: on
