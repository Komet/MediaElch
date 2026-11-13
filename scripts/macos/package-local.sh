#!/usr/bin/env bash
#
# Packages a local macOS build into a self-contained app bundle.  It is
# architecture-native, i.e. arm64 on Apple silicon, and is intended for local
# use, not for official releases.
#
# The script does not depend on a specific package manager.  Locations of Qt,
# MediaInfo and ffmpeg can be set via environment variables:
#
#   QT_BIN_DIR      Directory containing qtpaths and macdeployqt
#   MEDIAINFO_LIB   Path to libmediainfo.0.dylib
#   FFMPEG_BIN      Path to the ffmpeg executable
#
# If they are not set, the script tries Homebrew, then MacPorts, then $PATH.
#
# Example (Homebrew):
#   brew install cmake ninja qt libmediainfo ffmpeg
#
# Example (MacPorts):
#   sudo port install cmake ninja qt6-qtbase qt6-qtmultimedia qt6-qtsvg \
#     qt6-qt5compat libmediainfo ffmpeg
#
# The Qt build may require a newer macOS version than MediaElch's default.
# Set CMAKE_OSX_DEPLOYMENT_TARGET accordingly when configuring, e.g.:
#   cmake --preset release -DCMAKE_PREFIX_PATH="<qt-prefix>" \
#     -DCMAKE_OSX_DEPLOYMENT_TARGET=14.4
#   cmake --build build/release
#
# Usage: scripts/macos/package-local.sh [build-directory]
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$PROJECT_DIR"

BUILD_DIR="${1:-$PROJECT_DIR/build}"

have_brew() { command -v brew > /dev/null 2>&1; }

if [[ -z "${QT_BIN_DIR:-}" ]]; then
  if have_brew && brew --prefix qt > /dev/null 2>&1; then
    QT_BIN_DIR="$(brew --prefix qt)/bin"
  elif [[ -x /opt/local/libexec/qt6/bin/macdeployqt ]]; then
    QT_BIN_DIR=/opt/local/libexec/qt6/bin
  elif command -v macdeployqt > /dev/null 2>&1; then
    QT_BIN_DIR="$(dirname "$(command -v macdeployqt)")"
  else
    echo "Could not find Qt; set QT_BIN_DIR." >&2
    exit 1
  fi
fi

if [[ -z "${MEDIAINFO_LIB:-}" ]]; then
  for candidate in \
      "$(have_brew && brew --prefix libmediainfo 2> /dev/null)/lib/libmediainfo.0.dylib" \
      /opt/local/lib/libmediainfo.0.dylib; do
    if [[ -f "$candidate" ]]; then
      MEDIAINFO_LIB="$candidate"
      break
    fi
  done
  if [[ -z "${MEDIAINFO_LIB:-}" ]]; then
    echo "Could not find libmediainfo.0.dylib; set MEDIAINFO_LIB." >&2
    exit 1
  fi
fi

if [[ -z "${FFMPEG_BIN:-}" ]]; then
  FFMPEG_BIN="$(command -v ffmpeg || true)"
  if [[ -z "$FFMPEG_BIN" ]]; then
    echo "Could not find ffmpeg; set FFMPEG_BIN." >&2
    exit 1
  fi
fi

echo "Qt:        $QT_BIN_DIR"
echo "MediaInfo: $MEDIAINFO_LIB"
echo "ffmpeg:    $FFMPEG_BIN"

cmake --build "$BUILD_DIR" --parallel 8

PACKAGE_DIR="$(mktemp -d "$BUILD_DIR/package-$(uname -m).XXXXXX")"
APP="$PACKAGE_DIR/MediaElch.app"

ditto "$BUILD_DIR/MediaElch.app" "$APP"

cp "$MEDIAINFO_LIB" "$APP/Contents/MacOS/libmediainfo.0.dylib"
cp "$FFMPEG_BIN" "$APP/Contents/MacOS/ffmpeg"

QT_TRANSLATIONS="$("$QT_BIN_DIR/qtpaths" --query QT_INSTALL_TRANSLATIONS)"
mkdir -p "$APP/Contents/translations"
cp "$QT_TRANSLATIONS"/qt*.qm "$APP/Contents/translations/"

"$QT_BIN_DIR/macdeployqt" "$APP" \
  -executable="$APP/Contents/MacOS/ffmpeg" \
  -executable="$APP/Contents/MacOS/libmediainfo.0.dylib" \
  -verbose=1

install_name_tool -id '@loader_path/libmediainfo.0.dylib' \
  "$APP/Contents/MacOS/libmediainfo.0.dylib"

python3 "$SCRIPT_DIR/check_bundle_dependencies.py" "$APP"
python3 "$SCRIPT_DIR/set_minimum_macos_version.py" \
  "$BUILD_DIR/CMakeCache.txt" "$APP/Contents/Info.plist"

codesign --force --deep --sign - "$APP"
codesign --verify --deep --strict "$APP"

"$APP/Contents/MacOS/ffmpeg" -version > "$PACKAGE_DIR/ffmpeg-version.txt"

printf '\nPackaged application:\n%s\n' "$APP"
printf '\nBuilt from commit:\n'
git rev-parse HEAD
