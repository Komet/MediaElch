# https://github.com/cheshirekow/cmake_format
find_program(
  CMAKE_FORMAT_EXE
  NAMES cmake-format
  PATHS /usr/bin/
  PATH_SUFFIXES bin
  DOC "cmake formater"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(CMakeFormat DEFAULT_MSG CMAKE_FORMAT_EXE)

mark_as_advanced(CMAKE_FORMAT_EXE)
