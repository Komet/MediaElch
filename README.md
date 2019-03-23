<div align="center">
	<img alt="MediaElch Logo" src="data/img/MediaElch.png" />
</div>

[![Build Status](https://travis-ci.org/Komet/MediaElch.svg?branch=master)](https://travis-ci.org/Komet/MediaElch)

# MediaElch

MediaElch is a MediaManager for Kodi. Information about Movies, TV Shows, Concerts and Music are stored as `nfo` files.
Fanarts are downloaded automatically from fanart.tv.
Using the `nfo` generator, MediaElch can be used with other MediaCenters as well.

Documentation can be found at https://mediaelch.github.io/mediaelch-doc/index.html


## Supported scrapers

Included are scrapers for The Movie DB, The TV DB, Videobuster, OFDb, IMDB and Fanart.tv (and also some adult content scrapers).
The Movie DB and The TV DB are available in more than 20 languages.
Music information is scraped from The Audio DB, All Music and Discogs.
Tv themes and trailers can also be downloaded.


## Download

Please visit https://mediaelch.github.io/mediaelch-doc/download.html


## Developer
For build instructions, see: https://mediaelch.github.io/mediaelch-doc/contributing/build.html

### Documentation

**User Documentation**

```sh
git submodule update --init
mkdir build && cd $_
cmake ..
make docs
```

**Doxygen**

```sh
mkdir build && cd $_
cmake ..
make doxygen
```

### Testing
See [test/README.md](./test/README.md).

### Sanitizer

```sh
mkdir build && cd $_
qmake CONFIG += sanitize ..
make -j4
./mediaelch
```
