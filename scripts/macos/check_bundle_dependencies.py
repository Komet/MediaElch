#!/usr/bin/env python3
"""Verify that no Mach-O binary in an app bundle links against libraries
outside of the bundle (e.g. from Homebrew, MacPorts or a user directory).

Usage: check_bundle_dependencies.py path/to/MediaElch.app
"""
from pathlib import Path
import subprocess
import sys

EXTERNAL_PREFIXES = ("/opt/homebrew/", "/opt/local/", "/usr/local/", "/Users/")


def main() -> int:
    if len(sys.argv) != 2:
        sys.exit(f"Usage: {sys.argv[0]} path/to/App.app")

    root = Path(sys.argv[1])
    problems = []
    count = 0

    for path in root.rglob("*"):
        if not path.is_file() or path.is_symlink():
            continue
        kind = subprocess.check_output(["file", "-b", str(path)], text=True)
        if "Mach-O" not in kind:
            continue
        count += 1
        ids = subprocess.run(
            ["otool", "-D", str(path)], capture_output=True, text=True, check=True
        ).stdout.splitlines()[1:]
        ids = {line.strip() for line in ids}
        links = subprocess.check_output(["otool", "-L", str(path)], text=True)
        for line in links.splitlines()[1:]:
            dep = line.strip().split(" (compatibility version", 1)[0]
            if dep in ids:
                continue
            if dep.startswith(EXTERNAL_PREFIXES):
                problems.append(f"{path.relative_to(root)}: {dep}")

    print(f"{count} binaries checked.")
    if problems:
        print("\n".join(problems))
        sys.exit("Packaging aborted: external dependencies remain.")
    print("Dependency path check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
