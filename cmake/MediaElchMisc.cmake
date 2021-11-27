function(mediaelch_check_compiler_ids)
  if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
    message(
      WARNING
        "Different compilers for C++ (${CMAKE_CXX_COMPILER_ID}) and C (${CMAKE_C_COMPILER_ID})!"
    )
  endif()
endfunction()

# -----------------------------------------------------------------------------
# Set a default build type if none was specified
function(mediaelch_set_default_build_type)
  set(MEDIAELCH_DEFAULT_BUILD_TYPE "RelWithDebInfo")

  # Git project? Most likely a development environment
  if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set(MEDIAELCH_DEFAULT_BUILD_TYPE "Debug")
  endif()

  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(
      STATUS
        "Setting build type to '${MEDIAELCH_DEFAULT_BUILD_TYPE}' as none was specified."
    )
    set(CMAKE_BUILD_TYPE
        "${MEDIAELCH_DEFAULT_BUILD_TYPE}"
        CACHE STRING "Choose the type of build." FORCE
    )
    # Set the possible values of build type for cmake-gui
    set_property(
      CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel"
                                      "RelWithDebInfo"
    )
  endif()
endfunction()

# -----------------------------------------------------------------------------
# Some defaults for our targets. Currently warnings are enabled and the C++
# standard is set to C++14 (or 17 for Qt6). It simplifies handling multiple
# targets like different libraries without having to repeat all
# compile-features, etc.
function(mediaelch_post_target_defaults target)
  if(NOT TARGET ${target})
    message(WARNING "MediaElch defaults: ${target} is not a target.")
    return()
  endif()
  if(NOT Qt6_FOUND)
    target_compile_features(${target} PUBLIC cxx_std_14)
  else()
    target_compile_features(${target} PUBLIC cxx_std_17)
  endif()
  target_include_directories(
    ${target} PUBLIC "${CMAKE_BINARY_DIR}" "${CMAKE_SOURCE_DIR}"
                     "${CMAKE_SOURCE_DIR}/src"
  )
  enable_warnings(${target})
  target_enable_coverage(${target})
  add_sanitizers(${target})
  if(ENABLE_LTO)
    set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
  if(USE_EXTERN_QUAZIP)
    target_compile_definitions(${target} PRIVATE EXTERN_QUAZIP)
  endif()
  if(NOT DISABLE_UPDATER)
    target_compile_definitions(${target} PRIVATE MEDIAELCH_UPDATER)
  endif()
endfunction()
