# Documentation

The `docs/` directory is designated to contain project documentation.

## User Documentation

### Build User Documentation

User documentation can be found in `./user` and is created using Sphinx.

You need to have Sphinx and the
[Read the Docs Sphinx Theme](https://github.com/rtfd/sphinx_rtd_theme)
installed. You have two options to build the end-user documentation.
Either use the provided CMake target `docs` or use Sphinx's Makefile.

```sh
# Create user documentation using Sphinx's Makefile
cd docs/user/docs
make html
```

### [Maintainers Only] Publish User Documentation

Run: `./docs/user/update_github_pages.sh`.  
You can find our documentation on: <http://mediaelch.github.io/mediaelch-doc/>


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
