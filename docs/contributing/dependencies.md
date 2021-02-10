# MediaElch's Dependencies

MediaElch has a few dependencies. This document describes which
dependencies are used and why they are necessary (i.e. for what they are used).

## Runtime Dependencies

These dependencies are required to _run_ MediaElch.

For Windows, macOS and the Linux AppImage, they are bundled into the binary/download file.
Note that on Linux (not the AppImage, though) runtime dependencies are downloaded
through the system's package manager so the used versions may be older than the ones
provided by MediaElch.

<table>
  <thead>
    <tr>
      <th>Dependency</th>
      <th>Version</th>
      <th>Description</th>
      <th>License</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>
        <a href="https://qt.io">Qt</a>
      </td>
      <td>>= 5.6</td>
      <td>
        Qt is the GUI framework upon which MediaElch is build.
      </td>
      <td>
        <a href="https://www.qt.io/licensing/">LGPLv3</a>
      </td>
    </tr>
    <tr>
      <td><a href="https://ffmpeg.org/">ffmpeg</a></td>
      <td></td>
      <td>
        ffmpeg is used to create screenshots of movies.
        That's it!
        For all systems except Linux we use the latest version available.</td>
      <td>
        <a href="https://ffmpeg.org/legal.html">LGPLv2.1 / GPL 2 or later</a>
      </td>
    </tr>
    <tr>
      <td>
        <a href="https://github.com/MediaArea/MediaInfoLib">MediaInfoLib</a>
      </td>
      <td>>v20</td>
      <td>
        MediaInfo is used to gather video file details like the resolution, etc.
        For all systems except Linux we use the latest version available and only support MediaInfoLib versions that are available on the oldest still supported Ubuntu LTS release.
      </td>
      <td><a href="https://github.com/MediaArea/MediaInfoLib/blob/master/LICENSE">BSD 2-Clause License</a></td>
    </tr>
    <tr>
      <td><a href="https://github.com/MediaArea/ZenLib">Zenlib</a></td>
      <td></td>
      <td>
        ZenLib is a dependency of MediaInfoLib. MediaElch uses it for some string
   conversions that are related to MediaInfo.  
      </td>
      <td>
        <a href="https://github.com/MediaArea/ZenLib/blob/master/License.txt">
          zlib License
        </a>
      </td>
    </tr>
    <tr>
      <td><a href="https://github.com/stachenov/quazip">QuaZip</a></td>
      <td>0.9</td>
      <td>
        <p>
          A library which allows us to use Minizip with Qt.
          The latter is also part of Qt so we don't explicitly install it.
        </p>
        <p>
          Note that QuaZip v1 or later is not supported, yet.
        </p>
      </td>
      <td>
        <a href="https://github.com/stachenov/quazip/blob/v0.9.x/COPYING">
          LGPLv2.1 + static linking exception
        </a>
      </td>
    </tr>
  </tbody>
</table>

## Build Dependencies

These dependencies are either required for building the MediaElch binary or
for building and running our tests.

 - [CMake](https://cmake.org/)  
   CMake is our build system generator.
   It will replace QMake once our `CMakeLists.txt` is mature enough.
 - C++ Compiler (GCC/Clang/...)  
   For obvious reasons, we require a C++ compiler.  C++ compilers
   must support C++14 or higher.  In the future with Qt6, C++17 will
   be required.
 - [Catch2](https://github.com/catchorg/Catch2)  
   Catch2 is our testing framework.
   Please refer to our [Testing document](./testing.md) for more details.
 - QtTest  
   Some testing logic requires Qt's test module, e.g.
   [`QAbstractItemModelTester`](https://doc.qt.io/qt-5/qabstractitemmodeltester.html)

