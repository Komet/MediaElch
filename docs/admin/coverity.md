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
As of September 2019, this process is not yet integrated into MediaElch's CI
system. That may be implemented in the future.


## How to submit a new build?
Download coverity for C++ from https://scan.coverity.com/download
The next steps are shown in following bash script. You need to add Coverity's
`bin` directory to your `$PATH`.

```sh
export PATH="/path/to/coverity-analysis/bin/:$PATH"
git clone https://github.com/Komet/MediaElch.git
cd MediaElch
mkdir build && cd build
cmake .. -DUSE_EXTERN_QUAZIP=ON
cov-build --dir cov-int make -j 4
if grep "compilation units (100%) successfully" cov-int/build-log.txt; then
    tar caf myproject.xz cov-int
fi
```

We use the system's QuaZip so that it's code is not seen as part of MediaElch
by coverity. Maybe we should add it but as of now we assume that QuaZip is
fine.

Check that the Coverity build was successful and if it was then upload the
generated `myproject.xz` file to [Coverity][newcoverity].  Note that the
generated `.xz` file is multiple hundrets of megabytes large.


## How often should a new build be uploaded to Coverity?
Once a week would be optimal. But should be done at least once before a new
release to ensure that no obvious bugs are deployed.

## Who has write-access to Coverity?
The current Coverity-maintainer is GitHub user
[`bugwelle`](https://github.com/bugwelle/).


[synopsis]: https://www.synopsys.com/
[newcoverity]: https://scan.coverity.com/projects/komet-mediaelch/builds/new
