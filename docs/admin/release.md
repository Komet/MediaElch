# Release a new MediaElch version

This page and its children are used by MediaElch's maintainers and contain information
on how to release a new version, where to publish them, etc.

**If you are an enduser of MediaElch then please skip this part.**

Follow these steps when releasing a new version. Start from a fresh git repository:

```sh
git clone https://github.com/Komet/MediaElch.git
```


## Bump MediaElch Version

Change the version in following files:

 1. `Version.h`
 2. `MediaElch.plist`
 3. `obs/MediaElch.spec`
 4. `obs/README.md`


## Update Changelogs

 1. main changelog (`changelog.md`)
 2. debian changelog (use `dch -v "${ME_VERSION}-1" -D xenial -M`
 3. obs changelog (`obs/MediaElch.changes`)


## Update Git

 1. Commit your changes (MediaElch version and changelogs).
 2. Add a version tag and push your changes
 
  - `git tag -a v2.6.0 -m "MediaElch Version 2.6.0"`
  - `git push origin master`
  - `git push --tags`


## Package MediaElch for distributions

MediaElch is packaged for all major platforms. On Linux, we support multiple distributions.
Follow these steps to package a release version for publishing. After publishing MediaElch
do not forget to update this documentation.

### Windows
Wait for travis-ci to finish and download the latest ZIP. This requires a Git tag to be present.
Rename the ZIP, upload it on GitHub [Releases](https://github.com/Komet/MediaElch/releases).

### macOS
Same as for Windows. Download the latest `.dmg`, rename it and upload the version on
[GitHub Releases](https://github.com/Komet/MediaElch/releases).

### Debian
Releases for Debian and Ubuntu are distributed using [Launchpad](https://launchpad.net/).
What you need:

 1. Create a Launchpad account
 2. Add a GPG key
 3. Request membership for the [MediaElch Team](https://launchpad.net/~mediaelch)

MediaElch provides a simple script for releasing a new MediaElch debian package.  
See: https://github.com/Komet/MediaElch/blob/master/scripts/packaging/package.sh

```sh
# Create temporary directory
mkdir mediaelch-deb && cd $_
# Set your signing key (if it's not the same as in debian/changelog)
ME_SIGNING_KEY=D507F8C2686456B1B267B59C95D3C009C530B63C
# Have a clean repository
git clone https://github.com/Komet/MediaElch.git
cd MediaElch
./scripts/packaging/package.sh linux launchpad
```

### openSUSE
Releases for openSUSE are distributed using the [open build service](https://openbuildservice.org/).
See: https://github.com/Komet/MediaElch/blob/master/obs/README.md


## Publish Release Notes in Forums
A new release should be announced in some forum posts:

 - [English] https://forum.kodi.tv/showthread.php?tid=136333
 - [German] https://www.kodinerds.net/index.php/Thread/14560-MediaElch-MediaManager-for-Mac-Linux-Win/?pageNo=34


## Update External Documentation
There are several external forum posts and other documentation pages that may
need to be updated. Look at following pages and update them if neccessary:

 - [English] User documentation: https://github.com/mediaelch/mediaelch-doc
 - [English] https://kodi.wiki/view/MediaElch
 - [German] https://wiki.ubuntuusers.de/MediaElch
