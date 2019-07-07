# Original from https://github.com/cginternals/cmake-
# init/blob/master/cmake/Coverage.cmake

# Modified
include(${CMAKE_CURRENT_LIST_DIR}/Gcov.cmake)

set(COVERAGE_ENABLED OFF BOOL)

set(
  LCOV_EXCLUDE_COVERAGE
  ${LCOV_EXCLUDE_COVERAGE}
  "\"*/build/*\""
  "\"*/googletest/*\""
  "\"*v1*\""
  "\"/usr/*\""
  "\"*/external/*\""
  "\"*/third_party/*\""
)

# Function to register a target for enabled coverage report. Use this function
# on test executables.
function(generate_coverage_report target)
  if(${COVERAGE_ENABLED})
    generate_lcov_report(coverage-${target} ${target} ${ARGN})
    add_dependencies(coverage coverage-${target})
  endif()
endfunction()

# Enable or disable coverage. Sets COVERAGE_ENABLED which is used by
# target_enable_coverage
function(activate_coverage status)
  if(NOT ${status})
    set(COVERAGE_ENABLED ${status} PARENT_SCOPE)
    message(STATUS "Coverage lcov skipped: Manually disabled")
    return()
  endif()

  find_package(lcov)

  if(NOT lcov_FOUND)
    set(COVERAGE_ENABLED OFF PARENT_SCOPE)
    message(STATUS "Coverage lcov skipped: lcov not found")
    return()
  endif()

  set(COVERAGE_ENABLED ${status} PARENT_SCOPE)
  message(STATUS "Coverage report enabled")
endfunction()

# Add compile/linker flags to the target for code coverage. Requires
# COVERAGE_ENABLED to be ON.
function(target_enable_coverage target)
  if(NOT TARGET ${target})
    message(WARNING "target_enable_coverage: ${target} is not a target.")
    return()
  endif()

  if(${COVERAGE_ENABLED})
    if(
      "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
      OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"
    )
      target_compile_options(${target} PRIVATE -g --coverage)
    endif()

    if(
      "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
      OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux"
    )
      target_link_libraries(${target} PRIVATE --coverage)
    endif()

  endif()
endfunction()
