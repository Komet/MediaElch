# Build Qt6 with ThreadSanitizer

__Status__: This guide is work-in-progress!

If you build MediaElch with ThreadSanitizer (TSAN) enabled, you will likely
run into issues immediately.  The reason is that Qt's Mutexes and other thread
classes don't have markers required by TSAN.  Only with Qt 6.4, these markers
were added.

This guide may help you to build Qt6 with TSAN enabled.

Note that it's written by the current MediaElch maintainer and the instructions
may differ on your system.  I'm using Kubuntu 20.04.

The guide is based on this [QtForum answer].


## Build Directory

Let's have all files in a custom workspace.

```sh
mkdir -p "${HOME}/Projects/GitHub/qt6_dev"
cd "${HOME}/Projects/GitHub/qt6_dev"
PROJECT_DIR="$(pwd)"
```


## Install Dependencies

The configuration step below may tell you what you need to build certain
modules such as QtPDF.  These dependencies were not installed on my system,
so I had to install them.  There are probably a lot more.

See also <https://doc.qt.io/qt-6/linux-requirements.html>.

```sh
# System dependencies
python3 -m pip install --user html5lib # for QtWebEngine and QtPdf
sudo apt install libnss3-dev libx11-xcb-dev libcups2-dev
```


## Build GLib

We build our own GLib with TSAN enabled.  This was recommended by the
Forum post and I didn't try it without.

```sh
# Build GLIB with TSAN enabled
cd "${PROJECT_DIR:?}"
git clone https://gitlab.gnome.org/GNOME/glib.git
cd glib
git checkout 2.75.0
cd ..
mkdir glib-build
cd glib-build
meson setup ../glib -Dbuildtype=debug -Db_sanitize=thread --prefix "${PROJECT_DIR}/glib-install"
meson compile
meson install
```


## Build Qt6

First, clone and initialize the Qt6 source code.
The Qt6 source code is huge, so cloning and initializing all submodules
may take a while.

```sh
# Qt6
cd "${PROJECT_DIR:?}"
git clone git://code.qt.io/qt/qt5.git qt6
cd qt6
perl init-repository
git checkout v6.4.1
cd ..
```

Now configure Qt6.  If you get a CMake error in regards to a function
`check_for_ulimit`, simply copy the function to `qtwebengine/configure.cmake`.

```sh
mkdir qt6-build
cd qt6-build
PKG_CONFIG_PATH="${PROJECT_DIR:?}/glib-install/lib/x86_64-linux-gnu/pkgconfig" \
  ../qt6/configure \
  -gui \
  -widgets \
  -accessibility \
  -skip qtactiveqt,qtquick3d,qtlanguageserver \
  -submodules qt5compat,qtbase,qtdeclarative,qthttpserver,qtimageformats,qtmultimedia,qtshadertools,qtnetworkauth,qtsvg,qttools,qttranslations,qtwebengine,qtxmlpatterns \
  -debug \
  -opensource \
  -confirm-license \
  -nomake examples \
  -nomake tests \
  -sanitize thread \
  -prefix "${PROJECT_DIR:?}/qt6-install"
```

Check the output! Also look at the configuration summary!
Ensure that all modules are configured correctly and to your liking!

```sh
cat "${PROJECT_DIR:?}/qt6-build/config.summary"
```

Now let's build Qt6 with all modules we selected.  Building on my system with
a AMD Ryzen 9 CPU took more than 2h.  At peak it took all of my 30 GiB RAM
plus 30 GiB Swap.  All files are about 80 GiB!

```sh
cmake --build . --parallel 24
cmake --install .
```

## Build MediaElch

```sh
export PATH="${HOME}/Projects/GitHub/qt6_dev/qt6-install:$PATH"
cmake -S . -B build/qt6  -DCMAKE_BUILD_TYPE=Debug -DSANITIZE_THREAD=ON -GNinja
cmake --build build/qt6

# Run
./build/qt6/MediaElch -platform minimal
```

## TODO

- [ ] xcb platform plugin


[QtForum answer]: https://forum.qt.io/topic/140544/qt-6-4-built-with-tsan-but-how-to-add-my-own-glib-build/3
