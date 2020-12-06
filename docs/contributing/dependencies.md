# MediaElch's Dependencies

MediaElch has a few dependencies. This document describes what and why
dependencies are used.

Note that on Linux (not the AppImage, though) dependencies are downloaded
through the system's package manager so the used versions may be older.

 - [Qt > 5.6][qt]  
   Qt is the GUI framework upon which MediaElch is build.
 - [ffmpeg][ffmpeg]  
   ffmpeg is used to create screenshots of movies. That's it!
   For all systems except Linux we use the latest version available.
 - [MediaInfoLib][mediainfolib]  
   MediaInfo is used to gather video file details like the resolution, etc.
   For all systems except Linux we use the latest version available.
 - [Zenlib][zenlib]  
   ZenLib is a dependency of MediaInfoLib. MediElch uses it for some string
   conversions that are related to MediaInfo.
 - [QuaZip][quazip]  
   A library which allows us to use Minizip with Qt.
   The latter is also part of Qt so we don't explicitly install it.
 - [Catch2][catch]  
   Catch2 is our testing framework.
   Please refer to our [Testing document](./testing.md) for more details.

[qt]: https://qt.io
[catch]: https://github.com/catchorg/Catch2
[ffmpeg]: https://ffmpeg.org/
[mediainfolib]: https://github.com/MediaArea/MediaInfoLib
[zenlib]: https://github.com/MediaArea/ZenLib
[quazip]: https://github.com/stachenov/quazip
