# Packaging MediaElch

At the moment, the MediaElch maintainers use some custom scripts for packaging
MediaElch.  These scripts can be found in `.ci/`.

In the future we may use CMake with CPack for creating packages.
This document uses contains a few resources that can help in doing so.

*TODO*

## Resources

 - https://cliutils.gitlab.io/modern-cmake/chapters/install/packaging.html
 - https://gitlab.kitware.com/cmake/community/wikis/doc/cpack/Configuration

## Packaging with CMake and CPack

We use CPack as an experimental way to package MediaElch.

### Source Packaging

```sh
mkdir build && cd $_
cmake ..
cpack --config CPackSourceConfig.cmake .
```
