<div align="center">
	<img alt="MediaElch Logo" src="img/MediaElch.png" />
</div>

[![Build Status](https://travis-ci.org/Komet/MediaElch.svg?branch=master)](https://travis-ci.org/Komet/MediaElch)

MediaElch
=========

MediaElch is a MediaManager for Kodi. Information about Movies, TV Shows, Concerts and Music are stored as `nfo` files.
Fanarts are downloaded automatically from fanart.tv.
Using the `nfo` generator, MediaElch can be used with other MediaCenters as well.

Supported scrapers
------------------

Included are scrapers for The Movie DB, The TV DB, Kino.de, Videobuster, OFDb, IMDB and Fanart.tv (and also some adult content scrapers).
The Movie DB and The TV DB are available in more than 20 languages.
Music information is scraped from The Audio DB, All Music and Discogs.
Tv themes and trailers can also be downloaded.

Download
--------

Binaries are available for macOS, Linux ([AppImage](https://appimage.org/)) and Windows.
They can be downloaded at https://bintray.com/komet/MediaElch (choose your system -> click on tab "Files" -> select version).

| System           | Version           | Download           |
|------------------|-------------------|:------------------:|
| Windows          | latest (unstable) | [![Download](https://api.bintray.com/packages/komet/MediaElch/MediaElch-win/images/download.svg) ](https://bintray.com/komet/MediaElch/MediaElch-win/_latestVersion) |
| macOS            | latest (unstable) | [![Download](https://api.bintray.com/packages/komet/MediaElch/MediaElch-macOS/images/download.svg) ](https://bintray.com/komet/MediaElch/MediaElch-macOS/_latestVersion) |
| Linux (AppImage) | latest (unstable) | [![Download](https://api.bintray.com/packages/komet/MediaElch/MediaElch-linux/images/download.svg) ](https://bintray.com/komet/MediaElch/MediaElch-linux/_latestVersion) |

User Documentation
------------------

 - [FAQ](doc/FAQ.md)
 - [Advanced Settings](doc/AdvancedSettings.md)
 - [Export Theme](doc/ExportTheme.md)
 - [Renaming Files](doc/RenamingFiles.md)
 - [Synchronization with Kodi](doc/KodiSynchronization.md)
 - [Windows Portable Mode](doc/PortableMode.md)

Developer Documentation
------------------

**Doxygen**

```sh
cd doc
doxygen ./Doxyfile
```
