# Release a new MediaElch version

This page and its children are used by MediaElch's maintainers and contain information
on how to release a new version, where to publish them, etc.

**If you are an enduser of MediaElch then please skip this part.**

Follow these steps when releasing a new version. Start from a fresh git repository:

```sh
git clone https://github.com/Komet/MediaElch.git
```

## Update Translations

There may have been updates to MediaElch's translation files on transifex which
have not been inlcuded in the current master branch.

See [transifex.md](transifex.md)


## Bump MediaElch's Version

Change the version in following files:

 1. `CMakeLists.txt`
 1. `Version.h`
 1. `MediaElch.plist`
 1. `obs/MediaElch.spec`
 1. `obs/README.md`
 1. `.github/ISSUE_TEMPLATE/bug_report.md`
 1. `.github/ISSUE_TEMPLATE/scraper-does-not-work.md`

If done then search for the old version string to ensure that no other
file was missed. In the latter case, update the list above.


## Update Changelogs

 1. [main changelog](#user-content-notes--main-changelog) (`changelog.md`)
 2. [debian changelog](#user-content-notes--debian-changelog) (use `dch -v "${ME_VERSION}-1" -D xenial -M`)
 3. [obs changelog](#user-content-notes--obs-changelog) (`obs/MediaElch.changes`)

### Main Changelog
The main changelog should already contain all relevant changes because
they should have been added right with the corresponding commits.
But better check all commit messages since the last version tag:

```sh
# Print all commits between the git tag v2.8.7 and the current master branch
git log --oneline v2.8.7..master
# Count the number of commits since the last version
git log --oneline v2.8.7..master | wc -l
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
 
  - `git tag -a v2.8.7 -m "MediaElch Version 2.8.7"`
  - `git push origin master`
  - `git push --tags`


## Package MediaElch for distributions

MediaElch is packaged for all major platforms. On Linux, we support multiple distributions.
Follow these steps to package a release version for publishing. After publishing MediaElch
do not forget to update this documentation.

### Windows
Wait for travis-ci to finish and download the latest ZIP. This requires a Git tag to be present.
Rename the ZIP, upload it on GitHub [Releases](https://github.com/Komet/MediaElch/releases).

Notify the current maintainer of the [Chocolatey MediaElch package][choco].
The update process on their side includes updating MediaElch's version number in the
download URL and uploading it to Chocolatey. If feasible, you can also follow the steps in
the GitHub repository and create a PR that bumps the version (see URLs below).
It's important that the Windows ZIP is uploaded to a place that is reliable like
GitHub Releases or otherwise chocolatey will have issues.

Resources:
 - https://github.com/Komet/MediaElch/issues/544
 - https://github.com/mediaelch/chocolatey-mediaelch

[choco]: https://chocolatey.org/packages/MediaElch/

### macOS
Same as for Windows. Download the latest `.dmg`, rename it and upload the version on
[GitHub Releases](https://github.com/Komet/MediaElch/releases).

After that update MediaElch for Homebrew: https://formulae.brew.sh/cask/mediaelch
This includes creating a pull request with an updated `mediaelch.rb`.

### AppImage
Same as for Windows and macOS. Download the latest `.AppImage`, rename it and upload
the version on [GitHub Releases](https://github.com/Komet/MediaElch/releases).

### Debian
Releases for Debian and Ubuntu are distributed using [Launchpad](https://launchpad.net/).
What you need:

 1. Create a Launchpad account
 2. Add a GPG key
 3. Request membership for the [MediaElch Team](https://launchpad.net/~mediaelch)

MediaElch provides a simple script for releasing a new MediaElch debian package.  
See: https://github.com/Komet/MediaElch/blob/master/scripts/packaging/package.sh

In `scripts/packaging/package.sh` check if there are new Ubuntu versions and
update the function `package_and_upload_to_launchpad`. Pushing another distro
is rather cumbersome when using the script.  
See https://launchpad.net/ubuntu/+series for supported Ubuntu versions.

```sh
# Create temporary directory
mkdir mediaelch-deb && cd $_
# Set your signing key (if it's not the same as in debian/changelog)
export ME_SIGNING_KEY=66F39BA8CDE39366460D85F82BBFBFBFAE919C9F
# Either stable/nightly/test
export ME_LAUNCHPAD_TYPE=stable
# Have a clean repository
git clone https://github.com/Komet/MediaElch.git
cd MediaElch
.ci/linux/package_linux_launchpad.sh launchpad
```

Your GPG key may be outdated. Please see: <https://help.ubuntu.com/community/GnuPrivacyGuardHowto>

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
