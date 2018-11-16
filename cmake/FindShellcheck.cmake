find_program(
  SHELLCHECK_EXE
  NAMES shellcheck
  PATHS /usr/bin/
  PATH_SUFFIXES bin
  DOC "shellcheck shell linter"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Shellcheck DEFAULT_MSG SHELLCHECK_EXE)

mark_as_advanced(SHELLCHECK_EXE)
