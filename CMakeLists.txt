# cmake build of MediaElch

# Uncomment this to see all commands cmake actually executes
# set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project(
  mediaelch
  VERSION 2.12.1
  DESCRIPTION "Media Manager for Kodi"
  HOMEPAGE_URL "https://mediaelch.github.io/"
)

message("=> Project: ${PROJECT_NAME}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# -----------------------------------------------------------------------------
# Project configuration options. Sanitizer options are defined in the
# corresponding FindXX modules.
# cmake-format: off
option(ENABLE_CLANG_TIDY       "Analyze code with clang-tidy."                          OFF)
option(ENABLE_CLANG_TIDY_FIX   "Analyze code with clang-tidy and fix errors."           OFF)
option(ENABLE_COVERAGE         "Add coverage information to binaries (requires GCC)."   OFF)
option(ENABLE_COLOR_OUTPUT     "Force produce ANSI-colored output (GNU/Clang only)."     ON)
option(ENABLE_LTO              "Enable link-time-optimization. Increases link time."    OFF)
option(ENABLE_MOLD_LINKER      "Enable the mold linker by adding -fuse-ld=mold flag"    OFF)
option(ENABLE_TESTS            "Also build MediaElch's tests."                          OFF)
option(DISABLE_UPDATER         "Disable MediaElch's update check."                      OFF)
option(USE_EXTERN_QUAZIP       "Build against the system's quazip library."             OFF)
option(MEDIAELCH_FORCE_QT5     "Even if Qt6 can be found, only search for Qt5"          OFF)
option(MEDIAELCH_FORCE_QT6     "Even if Qt5 can be found, only search for Qt6"          OFF)
# cmake-format: on

if(ENABLE_MOLD_LINKER)
    add_link_options("-fuse-ld=mold")
endif()

include(MediaElchMisc)
mediaelch_set_default_build_type()
mediaelch_check_compiler_ids()

find_package(Sanitizers)
if(SANITIZE_ADDRESS)
    add_definitions("-DMEDIAELCH_USE_ASAN_STACKTRACE")
endif()

include(GNUInstallDirs) # For ${CMAKE_INSTALL_<DIR>} variables that are
                        # standardized
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

# ------------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON) # Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOUIC ON) # Create code from a list of Qt designer ui files
set(CMAKE_AUTORCC ON) # For .qrc files

# ------------------------------------------------------------------------------
# Qt5/6: You may need to set CMAKE_PREFIX_PATH e.g. to ~/Qt/5.11.2/gcc_64/
if(MEDIAELCH_FORCE_QT5 AND MEDIAELCH_FORCE_QT6)
  message( FATAL_ERROR  "You can't set both MEDIAELCH_FORCE_QT5 and MEDIAELCH_FORCE_QT6! "
                        "Check CMakeCache.txt in your build directory")
elseif(NOT MEDIAELCH_FORCE_QT5 AND NOT MEDIAELCH_FORCE_QT6)
  find_package( QT NAMES Qt6 Qt5 COMPONENTS Core NO_MODULE REQUIRED )
elseif(MEDIAELCH_FORCE_QT6)
  find_package( QT NAMES     Qt6 COMPONENTS Core NO_MODULE REQUIRED )
elseif(MEDIAELCH_FORCE_QT5)
  find_package( QT NAMES     Qt5 COMPONENTS Core NO_MODULE REQUIRED )
endif()

find_package(
  Qt${QT_VERSION_MAJOR} REQUIRED
  COMPONENTS
    Concurrent
    Core
    Gui
    Multimedia
    MultimediaWidgets
    Network
    OpenGL
    Sql
    Svg
    Widgets
    Xml
    LinguistTools
)

if(ENABLE_TESTS)
  message(STATUS "Tests enabled")
  find_package(Qt${QT_VERSION_MAJOR} QUIET REQUIRED COMPONENTS Test)
else()
  message(STATUS "Tests disabled")
endif()

# Min version required; keep in sync with MediaElch.plist
# Qt5 CI builds adapt MediaElch.plist before building.
if(Qt6_FOUND)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 11)
  find_package(Qt6 QUIET REQUIRED COMPONENTS Core5Compat)
else()
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
endif()

message(STATUS "Using Qt ${QT_VERSION}")

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
if(Qt6_FOUND)
  qt6_add_translation(MEDIAELCH_QM_FILES ${MEDIAELCH_TS_FILES})
else()
  qt5_add_translation(MEDIAELCH_QM_FILES ${MEDIAELCH_TS_FILES})
endif()

# Copy the i18n.qrc because all referenced files are resolved relatively to it.
configure_file(data/i18n.qrc ${CMAKE_BINARY_DIR} COPYONLY)

# -----------------------------------------------------------------------------
# Subdirectories and main executable

add_subdirectory(docs)
add_subdirectory(third_party EXCLUDE_FROM_ALL)

# -----------------------------------------------------------------------------
# QuaZip submodule: We need to decide on the version we want too use.
if(USE_EXTERN_QUAZIP)
  find_package(QuaZip-Qt${QT_VERSION_MAJOR} QUIET)
  if(NOT TARGET QuaZip::QuaZip)
    if(${QT_VERSION_MAJOR} GREATER 5)
      message(
        FATAL_ERROR
          "Could not find QuaZip 1.x using CMake. And can't use quazip5 as Qt 6 was requested!"
      )
    endif()
    message(STATUS "Using system's QuaZip 0.x library")
    add_library(MEDIAELCH_QUAZIP_TMP INTERFACE)
    # We assume that a library "quazip5" exists
    target_link_libraries(MEDIAELCH_QUAZIP_TMP INTERFACE quazip5)
    # We use this alias to still support QuaZip 0.9 which doesn't provide a
    # CMake config file which means we can't use find_package() on it.
    add_library(QuaZip::QuaZip ALIAS MEDIAELCH_QUAZIP_TMP)
  endif()
endif()

# -----------------------------------------------------------------------------
add_subdirectory(src)

add_executable(
  mediaelch MACOSX_BUNDLE ${CMAKE_BINARY_DIR}/i18n.qrc ${MEDIAELCH_QM_FILES} src/main.cpp
)

target_link_libraries(mediaelch PRIVATE libmediaelch)
set_target_properties(mediaelch PROPERTIES OUTPUT_NAME "MediaElch")
set_target_properties(mediaelch PROPERTIES CXX_VISIBILITY_PRESET hidden)
mediaelch_post_target_defaults(mediaelch)

if(APPLE)
  set_target_properties(mediaelch PROPERTIES
    BUNDLE True
    # Note: Plist does not contain placeholders, yet, because it's also used by QMake.
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.kvibes.MediaElch"
    MACOSX_BUNDLE_BUNDLE_NAME "MediaElch"
    MACOSX_BUNDLE_BUNDLE_VERSION "${mediaelch_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${mediaelch_VERSION}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/MediaElch.plist"
  )
  target_sources(mediaelch PRIVATE MediaElch.icns)
  set_source_files_properties(MediaElch.icns PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
endif()

# ------------------------------------------------------------------------------
# Other Config

# Used for easy version access.
configure_file(data/VERSION.txt.in VERSION.txt @ONLY)


# ------------------------------------------------------------------------------
# Installation
install(TARGETS mediaelch
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" RENAME MediaElch
  BUNDLE DESTINATION .)

if (NOT APPLE)
  install(FILES data/desktop/MediaElch.desktop DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
  install(FILES data/desktop/MediaElch.png DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps")
  install(
    FILES data/desktop/com.kvibes.MediaElch.metainfo.xml
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo"
  )
else()
  # Note Mac specific extension .app
  set(APPS "\${CMAKE_INSTALL_PREFIX}/MediaElch.app")
  # Directories to look for dependencies
  set(DIRS ${CMAKE_BINARY_DIR})
  install(CODE "include(BundleUtilities)
      fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")")
endif()

# ------------------------------------------------------------------------------
# Testing
if(ENABLE_TESTS)
  include(CTest)
  include(Catch)
  enable_testing() # Per CMake documentation, enable_testing() must be called in
  # the root directory.
  add_subdirectory(test)
endif()

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
set(CPACK_PACKAGE_FILE_NAME         "mediaelch-${PROJECT_VERSION}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME  "mediaelch-${PROJECT_VERSION}")

set(CPACK_PACKAGE_EXECUTABLES "mediaelch;MediaElch") # MediaElch is an alias for
                                                     # mediaelch. Used by NSIS

# Ignore these files when creating a source package.
# Essentially just our .gitignore and a few other files.
set(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/"      # We don't need git files
                              "/\\\\.idea/"     # IDEs
                              "/\\\\.vscode/"
                              "/\\\\.gdb/"
                              "/\\\\.venv/"
                              "/\\\\.tx/"
                              "\\\\.swp$"
                              "\\\\.DS_Store$"
                              "/\\\\.ci/"       # CI files
                              "Jenkinsfile$"       # CI files
                              "/build.*/"       # Any build executables
                              "/cmake-build-debug/"
                              "/ZenLib/"
                              "Makefile$"
                              ".*\\\\.log$"
                              "/MediaInfoDLL/"
                              "/scripts/"       # scripts and generated data
                              "/\\\\.github/"   # issue templates, etc.
                              "\\\\.#"
                              ".*\\\\.AppImage$"
                              ".*\\\\.zip$"
                              ".*\\\\.dmg$"
                              ".*\\\\.app$"
                              ".*\\\\.tar\\\\.gz$"
                              ".*\\\\.7z$"
                              ".*\\\\.user.*"
                              "\\\\.gdbinit"
                              "\\\\.clang-tidy"
                              "\\\\.clang-format"
                              "/obs/"                      # Packaging
                              "/tmp/"
                              "/debian/"
                              "/third_party/packaging_.*/"
                              "/#"
                              ".*~")
# cmake-format: on

if (NOT CPACK_GENERATOR)
  if(APPLE)
    set(CPACK_GENERATOR "DragNDrop;Bundle")
  elseif(WIN32)
    set(CPACK_GENERATOR "ZIP")
  else()
    set(CPACK_GENERATOR "TGZ")
  endif()
endif()

if(WIN32 AND NOT CPACK_SOURCE_GENERATOR)
  set(CPACK_SOURCE_GENERATOR "ZIP")
else()
  set(CPACK_SOURCE_GENERATOR "TGZ")
endif()

include(CPack)
