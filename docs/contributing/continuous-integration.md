# Continuous Integration (CI) and Continuous Delivery (CD)

__State__: last updated 2022-12-22

## What is CI?

See [Wikipedia][wikipedia_ci]. For MediaElch this means that we have one main code
branch (`master`) and each code change (i.e. Git commit) is build and
tested automatically by a CI service.
Furthermore, every pull request is tested and will only be merged if all tests succeed.

In combination with CI, we can push our test coverage to Coverity.


## What is CD?

The term is often used in conjunction with webservices.  When code is changed, tests
are run automatically and if they succeed then the application is deployed automatically
to production systems (*Note:* There are many definitions of CD and this one is incomplete).

For MediaElch, this means that every code change to the `master` branch is build by one
CI service and is then deployed to our [Nightly download page][download-page].
This way the latest change can always be tested by users and we don't
have to deploy anything by hand.


## What CI services do you use?

Since 2021-02-10 we use a private Jenkins instance.  Its scripts can be
found in `./.ci/jenkins/` of MediaElch's repository and in `Jenkinsfile`
in the root directory of MediaElch.


## What does Jenkins do?

The [`Jenkinsfile`](../../Jenkinsfile) file contains all steps that are run each time
a pull request is opened or a change is committed.  These include:

 1. Checks
    - `clang-format` for C++ formatting
    - `cmake-format` for CMake formatting
    - `cppcheck` for common C++ issues and bugs
    - `shellcheck` for linting Bash scripts
 2. Builds: Linux CI builds
 3. Tests: run unit and integration tests

If there are changes, once a day we run other builds that also deploy the
resulting binaries.  These include:

 1. Checks: same as above
 2. Builds
    - TODO: macOS build (latest Qt version using Homebrew)
    - Linux AppImage
    - Windows ZIP file (using `mxe`, which builds Windows executables on Linux)
 3. Tests: same as above
 4. Deployment
    - Successful builds are deployed to our [Nightly download page][download-page].


## History

From 2018-03-10 until 2020-12-19 MediaElch used TravisCI.  They heavily reduced their free
plan so that we were forced to drop TravisCI and switch to another CI runner.
TravisCI was used for uploading nightlies, uploading code coverage and more.  For historic
reasons, you can find the `.travis.yml` file [here][travis_old].

From 2015 until 2018, a private GitLab CI instance was used to deploy a nightly version of MediaElch.
You can find the deprecated CI script [here][GitlabKomet].


[download-page]: https://mediaelch-downloads.ameyering.de/snapshots/
[GitlabKomet]: https://github.com/Komet/MediaElch/blob/fafd391f83325a52a3e101684b88c7622cb085a0/.gitlab-ci.yml
[wikipedia_ci]: (https://en.wikipedia.org/wiki/Continuous_integration)
[travis_old]: https://github.com/Komet/MediaElch/blob/97bde70523ce7627a9c34503e45478b449ce93b0/.travis.yml
