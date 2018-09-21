<div align="center">
	<img alt="MediaElch Logo" src="img/MediaElch.png" />
</div>

[![Build Status](https://travis-ci.org/Komet/MediaElch.svg?branch=master)](https://travis-ci.org/Komet/MediaElch)

# MediaElch

MediaElch is a MediaManager for Kodi. Information about Movies, TV Shows, Concerts and Music are stored as `nfo` files.
Fanarts are downloaded automatically from fanart.tv.
Using the `nfo` generator, MediaElch can be used with other MediaCenters as well.

Documentation can be found at https://mediaelch.github.io/mediaelch-doc/index.html


## Supported scrapers

Included are scrapers for The Movie DB, The TV DB, Kino.de, Videobuster, OFDb, IMDB and Fanart.tv (and also some adult content scrapers).
The Movie DB and The TV DB are available in more than 20 languages.
Music information is scraped from The Audio DB, All Music and Discogs.
Tv themes and trailers can also be downloaded.


## Download

Binaries are available for macOS, Linux ([AppImage](https://appimage.org/)) and Windows.
They can be downloaded at https://bintray.com/komet/MediaElch (choose your system -> click on tab "Files" -> select version).

| System           | Version           | Download           |
|------------------|-------------------|:------------------:|
| Windows          | latest (unstable) | [![Download](https://api.bintray.com/packages/komet/MediaElch/MediaElch-win/images/download.svg) ](https://bintray.com/komet/MediaElch/MediaElch-win/_latestVersion) |
| macOS            | latest (unstable) | [![Download](https://api.bintray.com/packages/komet/MediaElch/MediaElch-macOS/images/download.svg) ](https://bintray.com/komet/MediaElch/MediaElch-macOS/_latestVersion) |
| Linux (AppImage) | latest (unstable) | [![Download](https://api.bintray.com/packages/komet/MediaElch/MediaElch-linux/images/download.svg) ](https://bintray.com/komet/MediaElch/MediaElch-linux/_latestVersion) |


## Developer

### Documentation

**Doxygen**

```sh
cd doc
doxygen ./Doxyfile
```

### Testing

Tests are written using [Catch2](https://github.com/catchorg/Catch2).

```sh
mkdir build && cd $_
qmake .. CONFIG+=test
make -j4
./mediaelch-test -h            # List Catch2 help
./mediaelch-test -t            # List all tags
./mediaelch-test -d yes        # Run *all* tests and print duration
./mediaelch-test "[load_data]" # Run scraping tests (online test)
./mediaelch-test "[search]"    # Run scraping search tests (online test)
```

### Sanitizer

```sh
mkdir build && cd $_
qmake CONFIG += sanitize ..
make -j4
./mediaelch
```
