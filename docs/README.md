# Documentation

The `docs/` directory is designated to contain project documentation.

## User Documentation

Documentation for users can be found at <https://mediaelch.github.io/mediaelch-doc/>.
It's source code can be found at <https://github.com/mediaelch/mediaelch-doc/>.


## Contributor / Developer Documentation

The `contribute` folder contains tutorials, guides, best-practices and design
documents for MediaElch. They are useful to developers and maintainers of
MediaElch.

See [`contributing/README.md`](contributing/README.md) for the table-of-contents.


### Doxygen

This project does not contain a `Doxyfile` as we have a custom CMake
target for this.

```sh
# Create Doxygen documentation using CMake
mkdir build && cd $_
cmake ..
make doxygen
```


## Maintainer Documentation

The `admin` directory contains a few documents that are useful to the
current MediaElch maintainers. They describe how to publish a new
MediaElch version, how to manage Transifex and so on.

May still be interesting to non-maintainers.  
See [`admin/README.md`](admin/README.md).
