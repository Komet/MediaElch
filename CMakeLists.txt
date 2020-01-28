# cmake build of MediaElch

# Please note that CMake support is experimental.

# Uncomment this to see all commands cmake actually executes
# set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(
  mediaelch
  VERSION 2.6.3
  DESCRIPTION "Media Manager for Kodi"
  HOMEPAGE_URL "https://mediaelch.github.io/"
)

# Don't process generated source files with AUTOMOC. New in 3.10
if(POLICY CMP0071)
  cmake_policy(SET CMP0071 OLD)
endif()

message("=> Project: ${PROJECT_NAME}")

# -----------------------------------------------------------------------------
# Set a default build type if none was specified
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

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# -----------------------------------------------------------------------------
# Project configuration options. Sanitizer options are defined in the
# correspondig FindXX modules.
# cmake-format: off
option(ENABLE_CLANG_TIDY       "Analyze code with clang-tidy."                       OFF)
option(ENABLE_CLANG_TIDY_FIX   "Analyze code with clang-tidy and fix errors."        OFF)
option(ENABLE_COVERAGE         "Add coverage information to binaries."               OFF)
option(ENABLE_COLOR_OUTPUT     "Force produce ANSI-colored output (GNU/Clang only)."  ON)
option(ENABLE_LTO              "Enable link-time-optimization. Increases link time." OFF)
option(DISABLE_UPDATER         "Disable MediaElch's update check."                   OFF)
option(USE_EXTERN_QUAZIP       "Build against the system's quazip library."          OFF)
# cmake-format: on

find_package(Sanitizers)
include(warnings)
include(coverage)
include(clang-tidy)
include(colors)

activate_coverage(ENABLE_COVERAGE)

# -----------------------------------------------------------------------------
# Optional IPO. Do not use IPO if it's not supported by compiler. IPO is
# interprocedural optimization (also known as link-time-optimization).
if(ENABLE_LTO)
  include(CheckIPOSupported)
  check_ipo_supported() # fatal error if IPO/LTO is not supported
  message(STATUS "Using LTO")
endif()

set(LINK_WHAT_YOU_USE ON)

# -----------------------------------------------------------------------------
# Some defaults for our targets. Currently warnings are enabled and the C++
# standard is set to C++14. It simplifies handling multiple targets like
# different libraries without having to repeat all compile-features, etc.
function(mediaelch_post_target_defaults target)
  if(NOT TARGET ${target})
    message(WARNING "MediaElch defaults: ${target} is not a target.")
    return()
  endif()
  target_compile_features(${target} PUBLIC cxx_std_14)
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

# ------------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON) # Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOUIC ON) # Create code from a list of Qt designer ui files
set(CMAKE_AUTORCC ON) # For .qrc files

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.7) # Min version required

# ------------------------------------------------------------------------------
# Qt5: You may need to set CMAKE_PREFIX_PATH e.g. to ~/Qt/5.11.2/gcc_64/
find_package(
  Qt5
  COMPONENTS
    Concurrent
    Core
    Gui
    Multimedia
    MultimediaWidgets
    Network
    OpenGL
    Qml
    Quick
    QuickWidgets
    Sql
    Widgets
    Xml
    REQUIRED
)

# -----------------------------------------------------------------------------
# Subdirectories and main executable

add_subdirectory(docs)
add_subdirectory(third_party EXCLUDE_FROM_ALL)
add_subdirectory(src)

add_executable(mediaelch src/main.cpp)
target_link_libraries(mediaelch PRIVATE libmediaelch)
set_target_properties(mediaelch PROPERTIES OUTPUT_NAME "MediaElch")
mediaelch_post_target_defaults(mediaelch)

# ------------------------------------------------------------------------------
# Installation
install(TARGETS mediaelch RUNTIME DESTINATION bin RENAME MediaElch)
install(FILES data/desktop/MediaElch.desktop DESTINATION share/applications)
install(FILES data/desktop/MediaElch.png DESTINATION share/pixmaps)

# ------------------------------------------------------------------------------
# Testing
include(CTest)
include(Catch)
enable_testing() # Per CMake documentation, enable_testing() must be called in
                 # the root directory.
add_subdirectory(test)

# ------------------------------------------------------------------------------
# Packaging

include(InstallRequiredSystemLibraries)

# As per https://cmake.org/cmake/help/latest/module/CPack.html Only set
# variables that don't have correct default options.

# cmake-format: off
set(CPACK_PACKAGE_NAME              "MediaElch")              # Use upper-case name
set(CPACK_PACKAGE_VENDOR            "kvibes")                 # Be consistent with e.g. the config directory
set(CPACK_PACKAGE_CONTACT           "info@andremeyering.de")  # Current maintainer
set(CPACK_PACKAGE_DESCRIPTION       "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MediaElch")
set(CPACK_PACKAGE_ICON              "${CMAKE_CURRENT_SOURCE_DIR}/MediaElch.ico")
set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_FILE_README      "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_WELCOME     "${CMAKE_CURRENT_SOURCE_DIR}/data/installer/welcome.txt")

set(CPACK_PACKAGE_EXECUTABLES "mediaelch;MediaElch") # MediaElch is an alias for
                                                     # mediaelch. Used by NSIS

# Ignore these files when creating a source package.
# Essentially just our .gitignore and a few other files.
set(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/"      # We don't need git files
                              "\\\\.swp$"
                              "\\\\.DS_Store$"
                              "/travis-ci/"     # Travis CI files
                              "/build.*/"       # Any build executables
                              "/ZenLib/"
                              "/MediaInfoDLL/"
                              "/docs/user/"     # Git submodule
                              "/scripts/"       # scripts and generated data
                              "/\\\\.github/"   # issue templates, etc.
                              "\\\\.#"
                              ".*AppImage$"
                              ".*\\\\.user.*"
                              "/#"
                              ".*~")
# cmake-format: on

if(APPLE AND NOT CPACK_GENERATOR)
  set(CPACK_GENERATOR "Bundle")
elseif(WIN32 AND NOT CPACK_GENERATOR)
  set(CPACK_GENERATOR "ZIP")
elseif(NOT CPACK_GENERATOR)
  set(CPACK_GENERATOR "TGZ")
endif()

if(WIN32 AND NOT CPACK_SOURCE_GENERATOR)
  set(CPACK_SOURCE_GENERATOR "ZIP")
else()
  set(CPACK_SOURCE_GENERATOR "TGZ")
endif()

include(CPack)
