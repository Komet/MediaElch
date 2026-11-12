#!/usr/bin/env python3
"""Set LSMinimumSystemVersion in an Info.plist to the CMAKE_OSX_DEPLOYMENT_TARGET
used for the build.

Usage: set_minimum_macos_version.py path/to/CMakeCache.txt path/to/Info.plist
"""
from pathlib import Path
import plistlib
import re
import sys


def main() -> int:
    if len(sys.argv) != 3:
        sys.exit(f"Usage: {sys.argv[0]} CMakeCache.txt Info.plist")

    cache = Path(sys.argv[1]).read_text()
    match = re.search(r"^CMAKE_OSX_DEPLOYMENT_TARGET:[^=]+=(.+)$", cache, re.MULTILINE)
    if not match:
        sys.exit("macOS deployment target missing from CMake cache.")
    target = match.group(1).strip()
    if not re.fullmatch(r"\d+(?:\.\d+){0,2}", target):
        sys.exit(f"Invalid macOS deployment target: {target}")

    path = Path(sys.argv[2])
    with path.open("rb") as handle:
        info = plistlib.load(handle)
    info["LSMinimumSystemVersion"] = target
    with path.open("wb") as handle:
        plistlib.dump(info, handle, sort_keys=False)
    print(f"Minimum bundle version: macOS {target}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
