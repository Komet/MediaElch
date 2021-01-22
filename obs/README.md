# MediaElch on openSUSE Build Service

This directory contains files needed to set up a project on
https://build.opensuse.org

*Note:* This is used to publish the release version of MediaElch.
Please use official MediaElch repositories instead of building
your own version.

*Note:* You need to replace `bugwelle` with your own username.

## Prerequisites

Please read https://openbuildservice.org/help/manuals/obs-beginners-guide/
and follow the examples. You need to have an account on
https://build.opensuse.org and must have the obs tool installed (e.g.
`sudo apt install osc obs-build` on Ubuntu).

You can also download the latest `osc` as described here:
https://en.opensuse.org/openSUSE:OSC  

## Steps to init a new repository on OBS
You should only need to do this *once*.

```sh
osc checkout home:bugwelle
cd home:bugwelle
osc mkpac MediaElch
cd MediaElch

# Copy MediaElch.changes and MediaElch.spec into this new directory
cp /path/to/MediaElch/obs/MediaElch.* .

# Package MediaElch into MediaElch-${ME_VERSION}.tar.gz 
# See "Compress MediaElch" for an example.

# Update MediaElch.changes => copy changelog.md
osc vc
osc add *.spec *.changes *.tar.gz
osc commit
```

## Steps to update a repository on OBS

```sh
# Only if MediaElch is not checked out
osc checkout home:bugwelle MediaElch
cd home:bugwelle/MediaElch

# Package MediaElch into MediaElch-${ME_VERSION}.tar.gz 
# See "Compress MediaElch" for an example.

# Delete the old .tar.gz
# Update MediaElch.spec

# Update MediaElch.changes => copy changelog.md
osc vc

osc addremove *.spec *.changes *.tar.gz
osc commit
```

## Compress MediaElch (`.tar.gz`)

```sh
rm MediaElch-*.tar.gz

export ME_VERSION=2.8.7
# Clone latest version.
git clone https://github.com/Komet/MediaElch.git MediaElch
cd MediaElch
git submodule update --init
cd ..
# Exclude .git, documentation and build folder.
# Reduces the tar.gz size from 27MB to 3MB
tar --exclude=MediaElch/.git \
	--exclude=MediaElch/scripts/generated_media \
	--exclude=MediaElch/docs \
	--exclude=MediaElch/build \
    -czf MediaElch-${ME_VERSION}.tar.gz MediaElch

cp MediaElch/obs/MediaElch.spec .
rm -rf MediaElch
```
