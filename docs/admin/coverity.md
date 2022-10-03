# Coverity Static Analysis

## What is Coverity?

Coverity is a static code analysis tool from [Synopsys][synopsis].
It can be used to scan projects for common security flaws
(e.g. `nullptr`-dereferencing for C++). It is free for open-source projects
and is used by Kodi and other big C++ projects.

Website: https://scan.coverity.com/


## How is it used in MediaElch?

Starting on 2019-09-11 the current maintainer should upload afresh project
[build][newcoverity] to the Coverity dashboard.
As of October 2022, this process is not yet integrated into MediaElch's CI
system. That may be implemented in the future.


## How to submit a new build?

Download coverity for C++ from <https://scan.coverity.com/download>
The next steps are shown in following bash script. You need to add Coverity's
`bin` directory to your `$PATH`.

```sh
# Note: You will need to adapt the path.
export PATH="${HOME}/Projects/Private/cov-analysis-linux64-2021.12.1/bin/:${PATH}"
git clone https://github.com/Komet/MediaElch.git
cd MediaElch
mkdir -p build/coverity && cd build/coverity
cmake -S ../.. -B . -DUSE_EXTERN_QUAZIP=ON -DENABLE_TESTS=ON
cov-build --dir cov-int make -j 12
if grep "compilation units (100%) successfully" cov-int/build-log.txt; then
    tar caf myproject.xz cov-int
fi
```

We use the system's QuaZip so that its code is not seen as part of MediaElch
by Coverity.  Maybe we should add it but as of now we assume that QuaZip is
fine.

Check that the Coverity build was successful and if it was then upload the
generated `myproject.xz` file to [Coverity][newcoverity].  Note that the
generated `.xz` file is multiple hundreds of megabytes large.


### Troubleshooting

It can happen that Coverity fails with e.g. only 3% of successful compilation
units.  The reason is unknown to me but I think it may have to do with custom
(more modern) GCC versions that I've installed locally that use more modern
standard library headers.  If it fails, try to build inside a docker container:

```sh
docker run -it -v "$(pwd):/opt" ubuntu:latest /bin/bash
```

You will need to install all of MediaElch's dependencies and then follow the
steps of the previous section.  You can't use MediaElch's CI image for Linux
because it also uses more recent compiler versions.


## How often should a new build be uploaded to Coverity?

Once a week or month would be optimal. But should be done at least once before
a new release to ensure that no obvious bugs are deployed.


## Who has write-access to Coverity?

The current Coverity-maintainer is GitHub user
[`bugwelle`](https://github.com/bugwelle/).


[synopsis]: https://www.synopsys.com/
[newcoverity]: https://scan.coverity.com/projects/komet-mediaelch/builds/new
