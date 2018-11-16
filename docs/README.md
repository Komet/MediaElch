# Documentation

The `docs/` directory is designated to contain project documentation.

## Build Documentation

User documentation can be found in `./user` and is created using Sphinx.
This project does not contain a `Doxyfile` as we have a custom CMake
target for this.

You need to have Sphinx and the
[Read the Docs Sphinx Theme](https://github.com/rtfd/sphinx_rtd_theme)
installed.

```sh
# Create user documentation using the Makefile
cd docs/user
make html

# Create Doxygen and Sphinx documentationUsing CMake
mkdir build && cd $_
cmake ..
make doxygen
make docs
```

## Publish User Documentation
On Linux run: `./user/update_github_pages.sh`.
You can find our documentation on: http://mediaelch.github.io/mediaelch-doc/
