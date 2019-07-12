# cmake build of MediaElch

# Please note that CMake support is experimental.

# Uncomment this to see all commands cmake actually executes
# set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(
  mediaelch
  VERSION 2.6.0
  DESCRIPTION "Media Manager for Kodi"
  HOMEPAGE_URL "http://mediaelch.github.io/"
)

# Don't process generated source files with AUTOMOC. New in 3.10
if(POLICY CMP0071)
  cmake_policy(SET CMP0071 OLD)
endif()

message("=> Project: ${PROJECT_NAME}")

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
    ${target}
    PUBLIC
      "${CMAKE_BINARY_DIR}" "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/src"
  )
  enable_warnings(${target})
  target_enable_coverage(${target})
  add_sanitizers(${target})
  if(ENABLE_LTO)
    set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
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
