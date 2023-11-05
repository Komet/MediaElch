#!/usr/bin/env python3

# Bump MediaElch's version.
# Takes care of updating the changelog, templates etc.
#
# Usage:
#   ./scripts/release/bump_version.py
#

import os
import sys
import re
import datetime
from collections import namedtuple

# Note: Version.h is handled specifically.

# Files with short-only MediaElch versions
files_current_short_only = [
    'CMakeLists.txt',
    'MediaElch.plist',
    'obs/MediaElch.spec',
    'obs/README.md',
    'docs/admin/release.md'
]
# Files with release and dev version string
files_release_and_dev = [
    '.github/ISSUE_TEMPLATE/bug_report.md',
    '.github/ISSUE_TEMPLATE/scraper-does-not-work.md',
]

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

if sys.version_info.major != 3 or sys.version_info.minor < 10 :
    eprint("Requires Python v3.10.0 or later")
    exit(1)


script_dir = os.path.dirname(os.path.realpath(__file__))
project_dir = os.path.realpath(os.path.join(script_dir, "../../"))
os.chdir(script_dir)

VersionName = namedtuple('VersionName', 'short long name is_dev dev release')

def is_uneven_patch(version):
    segments = version.split('.')
    last_segment = segments[-1]
    return int(last_segment) % 2 != 0

def increase_patch(version):
    segments = version.split('.')
    patch = int(segments[-1]) + 1
    return f"{segments[0]}.{segments[1]}.{patch}"

def decrease_patch(version):
    segments = version.split('.')
    patch = int(segments[-1]) - 1
    return f"{segments[0]}.{segments[1]}.{patch}"

def version_from_string(version, name):
    """
    From the given version short string (without suffix),
    create a VersionName tuple.  Calculates the current dev and
    release version, whether the version is a dev-version, etc.
    """
    is_dev = is_uneven_patch(version)

    if is_dev:
        dev = f"{version}-dev"
        version_long = dev
        release = decrease_patch(version)
    else:
        dev = increase_patch(version)
        dev = f"{dev}-dev"
        version_long = version
        release = version

    return VersionName(
        short = version, long = version_long,
        name = name, is_dev = is_dev,
        dev = dev, release = release)

def get_current_version_name():
    """
    From Version.h, get the current version string and name.
    Returns a VersionName which has all relevant details.
    """
    file = os.path.join(project_dir, "Version.h")

    version_short_re = r'AppVersionStr\[[^"]+"([^"]+)"'
    version_name_re = r'VersionName\[[^"]+"([^"]+)"'

    version_header = read_file(file)

    version_short = re.search(version_short_re, version_header).group(1)
    version_name = re.search(version_name_re, version_header).group(1)

    return version_from_string(version_short, version_name)


def request_new_version_name(old_name):
    new_short = input(f" - New short version ({old_name.short}): ")
    if not new_short:
        new_short = old_name.short

    new_name = input(f" - New version name ({old_name.name}): ")
    if not new_name:
        new_name = old_name.name

    return version_from_string(new_short, new_name)

def read_file(path):
    with open(path, 'r') as f:
        content = f.read()
    return content

def update_file(path, old_content, new_content):
    if old_content != new_content:
        print(f" - Updating version in: {path}")
        with open(path, 'w') as f:
            f.write(new_content)
    else:
        print(f" - Already up-to-date: {path}")

def replace_short_current_version(file, old_name, new_name):
    """
    Replace the version (without suffix) with the current one.
    Assumes that the given file does not use a version with suffix.
    """
    absolute_path = os.path.realpath(os.path.join(project_dir, file))

    content = read_file(absolute_path)

    new_content = content.replace(old_name.short, new_name.short)

    update_file(absolute_path, content, new_content)


def replace_release_nightly_versions(path, old_name, new_name):
    absolute_path = os.path.realpath(os.path.join(project_dir, file))

    content = read_file(absolute_path)

    new_content = content.replace(old_name.dev, new_name.dev)
    new_content = new_content.replace(old_name.release, new_name.release)

    update_file(absolute_path, content, new_content)

def replace_version_h(old_name, new_name):
    file = 'Version.h'
    absolute_path = os.path.realpath(os.path.join(project_dir, file))
    content = read_file(absolute_path)

    # Avoid replacing the long name with the short name: Only first result.
    new_content = content.replace(old_name.short, new_name.short, 1)
    new_content = new_content.replace(old_name.long, new_name.long)

    new_content = new_content.replace(old_name.name, new_name.name)

    update_file(absolute_path, content, new_content)

def update_changelog(version):
    absolute_path = os.path.join(project_dir, 'CHANGELOG.md')
    content = read_file(absolute_path)
    if not version.is_dev:
        date = datetime.date.today().strftime('%Y-%m-%d')
        changelog = content.replace('Upcoming', f"{version.short} - {date}")
        update_file(absolute_path, content, changelog)
    elif not '## Upcoming' in content:
        heading_start = content.find('##')
        upcoming = read_file(os.path.join(script_dir, 'changelog_placeholder.md'))
        changelog = content[:heading_start] + upcoming + content[heading_start:]
        update_file(absolute_path, content, changelog)

print("Update version strings in MediaElch source:")
old_version_name = get_current_version_name()
new_version_name = request_new_version_name(old_version_name)

print(f"""
Old: {old_version_name}
New: {new_version_name}
""")

for file in files_current_short_only:
    replace_short_current_version(file, old_version_name, new_version_name)

for file in files_release_and_dev:
    replace_release_nightly_versions(file, old_version_name, new_version_name)

replace_version_h(old_version_name, new_version_name)

update_changelog(new_version_name)

print("""
Done!
Now push your changes:

  git add .""")
if new_version_name.is_dev:
    print(f"  git commit -m 'release: Prepare next snapshot version v{new_version_name.long}'")
else:
    print(f"  git commit -m 'release: Release MediaElch v{new_version_name.short}'")

print("""  git push -u origin prepare-release
""")

if not new_version_name.is_dev:
    print("Also don't forget to push a snapshot version after the release!")
    print("")
