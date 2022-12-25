# Packaging

__State__: Early work-in-progress as of 2022-12-25.

This guide shows how packaging for certain platforms work.
For a detailed guide on how to make a release, see
“[Release a new MediaElch version](./release.md)”.


## Debian

### Getting Started with Debian Packaging

I found [this tutorial][PPA_Tutorial] to be a good start.  It is a all-in-one
tutorial.  Even though it does not go into full detail, I found it useful.

After that, I recommend the Debian guides:

 - [Debian New Maintainers' Guide](https://www.debian.org/doc/manuals/maint-guide/)
 - [Debian Manual: Source Packages](https://www.debian.org/doc/debian-policy/ch-source.html)

MediaElch's Debian packages are published on [launchpad.net].  This platform
builds sources packages, i.e. we don't publish binary packages but instead
ones containing MediaElch's source code which is then compiled.

Many tutorials, however, show you how to build binary packages, so keep that in
mind.


### Useful Packages / Tools

There are many useful tools provided by Debian to ease the process of packaging
software.  This sectionly only focuses on those that are useful for source
packages.  Tools such as `cowbuilder` for binary packages are not used.

- *`debhelper`*  
  This package includes tools such as `dh_make` and so forth.
  See `man debhelper` for more.


### Updating Debian packaging files

All files in `debian/` should be updated from time to time.  Here is a list of
things you should consider updating:

- `debian/control`  
  - [Standards-Version](https://www.debian.org/doc/debian-policy/upgrading-checklist.html)
  - `debhelper`: See `debian/compat` below.
  - `Build-Depends` and `Depends` should be tested once in a while:
    Old packages that are not available anymore can be removed.
    The alternative (indicated by `|`) can be used. Also check for later
    versions.

- `debian/compat`  
  Should contain the version of `debhelper` package available on the oldest
  Ubuntu version we support.


- `debian/` was created using `dh_make`


### TODO

 - https://salsa.debian.org/salsa-ci-team/pipeline / https://github.com/Komet/MediaElch/issues/1511#issuecomment-1363435155


[PPA_Tutorial]: https://saveriomiroddi.github.io/Building-a-debian-deb-source-package-and-publishing-it-on-an-ubuntu-ppa/
[launchpad.net]: https://launchpad.net/~mediaelch
