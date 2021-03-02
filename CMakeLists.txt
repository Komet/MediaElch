# cmake build of MediaElch

# Please note that CMake support is experimental.

# Uncomment this to see all commands cmake actually executes
# set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

project(
  mediaelch
  VERSION 2.8.7
  DESCRIPTION "Media Manager for Kodi"
  HOMEPAGE_URL "https://mediaelch.github.io/"
)

message("=> Project: ${PROJECT_NAME}")

if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "${CMAKE_CXX_COMPILER_ID}")
  message(
    FATAL_ERROR
      "Different compilers for C++ (${CMAKE_CXX_COMPILER_ID}) and C (${CMAKE_C_COMPILER_ID})!"
  )
endif()

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

# Min version required; keep in sync with MediaElch.plist
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)

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
    LinguistTools
  REQUIRED
)

# -----------------------------------------------------------------------------
# Translations

# Specify all *.ts files.
set(MEDIAELCH_TS_FILES
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_bg.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_cs_CZ.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_da.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_de.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_en.ts"
    # See https://github.com/Komet/MediaElch/issues/1191#issuecomment-789104632
    # Locale resolution is stupid...
    # "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_en_US.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_es_ES.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_fi.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_fr.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_it.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_ja.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_ko.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_nl_NL.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_no.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_pl.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_pt_BR.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_pt_PT.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_ru.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_sv.ts"
    "${CMAKE_SOURCE_DIR}/data/i18n/MediaElch_zh_CN.ts"
)
# Where to put the generated *.qm files.  Because MediaElch's data/i18n.qrc file
# uses relative paths, the structure in the build directory must be the same as
# specified in the i18n.qrc file.
set_source_files_properties(
  ${MEDIAELCH_TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_BINARY_DIR}/i18n"
)
qt5_add_translation(MEDIAELCH_QM_FILES ${MEDIAELCH_TS_FILES})
# Copy the i18n.qrc because all referenced files are resolved relatively to it.
configure_file(data/i18n.qrc ${CMAKE_BINARY_DIR} COPYONLY)

# -----------------------------------------------------------------------------
# Subdirectories and main executable

add_subdirectory(docs)
add_subdirectory(third_party EXCLUDE_FROM_ALL)
add_subdirectory(src)

add_executable(
  mediaelch ${CMAKE_BINARY_DIR}/i18n.qrc ${MEDIAELCH_QM_FILES} src/main.cpp
)

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
                              "/.ci/"           # CI files
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
