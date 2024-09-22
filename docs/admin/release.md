# Release a new MediaElch version

__State__: last updated 2023-11-05

This page is used by MediaElch's maintainers and contains information
on how to release a new version, where to publish them, etc.

**If you are an enduser of MediaElch then please skip this part.**

Follow these steps when releasing a new version. Start from a fresh git repository:

```sh
git clone https://github.com/Komet/MediaElch.git
```

## Update Translations

There may have been updates to MediaElch's translation files on Transifex which
have not been included in the current master branch.

See [transifex.md](transifex.md)


## Bump MediaElch's Version

There are many files which contain a version.  We have a script to update
them automatically. Simply run:

```sh
./scripts/release/bump_version.py
# … asks for new version, etc.
```

If done, search for the old version string to ensure that no other
file was missed.  If a file was missed, update the script above.


## Update Changelogs

 1. [main changelog](#main-changelog) (`changelog.md`)
 2. [debian changelog](#debian-changelog) (use `dch -v "${ME_VERSION}-1" -D xenial -M`)
 3. [obs changelog](#obs-changelog) (`obs/MediaElch.changes`)

### Main Changelog
The main changelog should already contain all relevant changes because
they should have been added right with the corresponding commits.
But better check all commit messages since the last version tag:

```sh
# Print all commits between a git tag and the current master branch
git log --oneline v2.12.0..master
# Count the number of commits since the last version
git log --oneline v2.12.0..master | wc -l
```


### Debian Changelog

TODO


### OBS Changelog

Don't put the full detailed changelog into the spec file. Only add "Updated MediaElch to vX.Y.Z".
Why? Because that's what the [rpm packaging guide][rpm-guide] tells us to:

> The last section, `%changelog` is a list of date-stamped entries that correlate to a
> specific Version-Release of the package.  This is not meant to be a log of what
> changed in the software from release to release, but specifically to packaging changes.

[rpm-guide]: https://rpm-guide.readthedocs.io/en/latest/rpm-guide.html#working-with-spec-files


## Update MediaElch's documentation

See https://github.com/mediaelch/mediaelch-doc  
Update the docs which includes:

 - update download URLs
 - update config file

Push your changes but do *not* yet upload the updated HTML pages to the `gh-pages` branch.
Then update the documentation submodule in the main repository so that when you
add a Git tag (see next section), it includes the latest documentation state.


## Update Git

 1. Commit your changes (MediaElch version and changelogs).
 2. Add a version tag and push your changes
 
  - `git tag -a v2.12.0 -m "MediaElch Version 2.12.0"`
  - `git push origin master` (or better: Create a pull request)
  - `git push --tags`
  - `git checkout release && git merge master && git push origin release`


## Package MediaElch for distributions

MediaElch is packaged for all major platforms. On Linux, we support multiple distributions.
Follow these steps to package a release version for publishing. After publishing MediaElch
do not forget to update this documentation.


### Windows

We build the Windows release using Docker and MXE.  This requires a Git tag to be present.

Run:
```sh
./.ci/win/build_windows_release_in_docker.sh
./.ci/win/package_windows_in_docker.sh
```

Rename the ZIP, upload it on GitHub [Releases](https://github.com/Komet/MediaElch/releases).

Notify the current maintainer of the [Chocolatey MediaElch package][choco].
Do so by opening an issue in [sumo300/chocolatey-packages-au](https://github.com/sumo300/chocolatey-packages-au/).
For an example see [here](https://github.com/sumo300/chocolatey-packages-au/issues/1).

_Background Info_  
The update process on their side includes updating MediaElch's version number in the
download URL and uploading it to Chocolatey.  If feasible, you can also follow the steps in
the GitHub repository and create a PR that bumps the version (see URLs below).
It's important that the Windows ZIP is uploaded to a place that is reliable like
GitHub Releases or otherwise chocolatey will have issues.

Resources:
 - https://github.com/Komet/MediaElch/issues/544
 - https://github.com/mediaelch/chocolatey-mediaelch (deprecated)


### macOS

Same as for Windows. Rename the `.dmg` and upload the version on
[GitHub Releases](https://github.com/Komet/MediaElch/releases).

After that update MediaElch for Homebrew: https://formulae.brew.sh/cask/mediaelch
This includes creating a pull request with an updated `mediaelch.rb`.


### AppImage

Same as for Windows and macOS. Run:

```sh
./.ci/linux/build_linux_release_in_docker.sh
./.ci/linux/package_linux_appimage_in_docker.sh
```

Rename the`.AppImage` and upload the version on
[GitHub Releases](https://github.com/Komet/MediaElch/releases).


### Debian

Releases for Debian and Ubuntu are distributed using [Launchpad](https://launchpad.net/).
What you need:

 1. Create a Launchpad account
 2. Add a GPG key
 3. Request membership for the [MediaElch Team](https://launchpad.net/~mediaelch)

MediaElch provides a simple script for releasing a new MediaElch Debian package.  
See: [`package_linux_launchpad.sh`](../../.ci/linux/package_linux_launchpad.sh)

In `.ci/linux/package_linux_launchpad.sh` check if there are new Ubuntu versions and
update the function `package_and_upload_to_launchpad`. Pushing another distro
is rather cumbersome when using the script.  
See https://launchpad.net/ubuntu/+series for supported Ubuntu versions.

```sh
# Create temporary directory
mkdir mediaelch-deb && cd mediaelch-deb
# Set your signing key (if it's not the same as in debian/changelog)
export ME_SIGNING_KEY="66F39BA8CDE39366460D85F82BBFBFBFAE919C9F"
# Either stable/nightly/test
export ME_LAUNCHPAD_TYPE="stable"
# Have a clean repository (full clone due to Git stuff in package*.sh script)
git clone https://github.com/Komet/MediaElch.git
cd MediaElch
.ci/linux/package_linux_launchpad.sh launchpad
```

Your GPG key may be outdated. Please see: <https://help.ubuntu.com/community/GnuPrivacyGuardHowto>


### openSUSE

Releases for openSUSE are distributed using the [open build service](https://build.opensuse.org/).
See [obs/README.md](https://github.com/Komet/MediaElch/blob/master/obs/README.md).

Check if the repository documentation needs to be updated (e.g. Leap 15.2 is outdated).


## Publish Release Notes in Forums

A new release should be announced in some forum posts:

 - [English] https://forum.kodi.tv/showthread.php?tid=136333
 - [German] https://www.kodinerds.net/index.php/Thread/14560-MediaElch-MediaManager-for-Mac-Linux-Win/?pageNo=34


## Update External Documentation

There are several external forum posts and other documentation pages that may
need to be updated. Look at following pages and update them if neccessary:

 - [English] User documentation: https://github.com/mediaelch/mediaelch-doc
 - [German] https://wiki.ubuntuusers.de/MediaElch

[choco]: https://chocolatey.org/packages/MediaElch/
