# Packaging MediaElch

__State__: *Work In Progress* as of 2022-12-22

Since MediaElch only supports a few package managers, other people
may want to package MediaElch for other systems.  This document tries
to give tips and help.


## Package Maintainer Notes

You want to package MediaElch?  That's great!  Here are some tips & tricks:

- __Use CMake__  
  While we still support QMake, that's only for historic reasons and
  because building MediaElch on Windows with CMake is (or was?) a pain.
  We still support QMake because it's bundled with Qt on many systems.

- __`DISABLE_UPDATER`__  
  This QMake and CMake option disables MediaElch's search for a new
  version on startup.  Since package managers should be used to determine
  the latest version, or the packaging guidelines say not to have
  "New version available" popups, you may want to disable our version-check.

- __`USE_EXTERN_QUAZIP`__  
  We use a version of QuaZip as a Git submodule.  This was done to
  make builds on Windows and macOS easier, but may be forbidden by your
  package manager guidelines.  Enable this CMake option to use your
  system's version of QuaZip.

- __`MEDIAELCH_FORCE_QT5`__, __`MEDIAELCH_FORCE_QT6`__  
  Use this CMake option if you have both Qt5 and Qt6 installed while
  packaging MediaElch, but want to enforce one of them.  Should not be
  required in CIs, but may be useful locally.


## Official Packages

At the moment, the MediaElch maintainers use some custom scripts for packaging
MediaElch.  These scripts can be found in `.ci/`.

In the future we may use CMake with CPack for creating packages.
This document uses contains a few resources that can help in doing so.

We package for:

- __Windows__  
  - A ZIP file with all dependencies is created.
  - A [Chocolatey] package is also created.
- __macOS__  
  A DMG file is created, i.e. an installer.
- __Linux__  
  - AppImage
  - PPA (Ubuntu)
  - RPM (openSUSE)


## Resources

 - <https://cliutils.gitlab.io/modern-cmake/chapters/install/packaging.html>
 - <https://gitlab.kitware.com/cmake/community/wikis/doc/cpack/Configuration>


## Packaging with CMake and CPack


We use CPack as an experimental way to package MediaElch.


### Source Packaging

```sh
mkdir build && cd $_
cmake ..
cpack --config CPackSourceConfig.cmake .
```

[Chocolatey]: https://community.chocolatey.org/packages/MediaElch

