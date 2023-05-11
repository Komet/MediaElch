# Documentation

The `docs/` directory is designated to contain project documentation.


## User Documentation

Documentation for users can be found at <https://mediaelch.github.io/mediaelch-doc/>.
Its source code can be found at <https://github.com/mediaelch/mediaelch-doc/>.


## Contributor / Developer Documentation

The `contributing` folder contains tutorials, guides and some best-practices
for MediaElch.  They are useful to developers and maintainers of MediaElch.

See [`contributing/README.md`](contributing/README.md) for the table-of-contents.
It also contains tips & tricks for your development setup.

See "Architecture Design Records" below for design documents.


### Doxygen

This project does not contain a `Doxyfile` as we have a custom CMake
target for this.

```sh
# Create Doxygen documentation using CMake
mkdir build && cd $_
cmake ..
make doxygen
```


## Architecture Design Records

Our [Architecture Design Records][adr] (ADR) describe design decision around our
architecture but also contains many TODO notes.  Our design documents are partially
moved there.

[adr]: https://github.com/joelparkerhenderson/architecture-decision-record


## Other Development Documents

There is also <https://github.com/mediaelch/mediaelch-dev>, which contains some
in-progress design documents and ideas.  It's a separate repository due to it containing
many (large) images that would bloat MediaElch's main repository.


## Maintainer Documentation

The `admin` directory contains a few documents that are useful to the
current MediaElch maintainers. They describe how to publish a new
MediaElch version, how to manage Transifex and so on.

It may still be interesting to non-maintainers.  
See [`admin/README.md`](admin/README.md).
