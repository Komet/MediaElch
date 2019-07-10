# Packaging MediaElch

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
