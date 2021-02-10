# Continuous Integration (CI) and Continuous Delivery (CD)

## What is CI?

See [Wikipedia][wikipedia_ci]. For MediaElch this means that we have one main code
branch (`master`) and each code change (i.e. Git commit) is build and tested automatically by a CI service.
Furthermore every pull request is tested and will only be merged if all tests succeed.

In combination with CI, we can push our test coverage to Coverity.


## What is CD?

The term is often used in conjunction with webservices.  When code is changed, tests
are run automatically and if they succeed then the application is deployed automatically
to production systems (*Note:* There are many definitions of CD and this one is incomplete).

For MediaElch, this means that every code change to the `master` branch is build by one
CI service and is then deployed to [Bintray][bintray] (see below).
This way the latest change can always be tested by users and we don't have to deploy anything by hand.

__Update__: JFrog will shut down Bintray. See <https://jfrog.com/blog/into-the-sunset-bintray-jcenter-gocenter-and-chartcenter/>.


## What CI services do you use?

From 2018-03-10 to 2020-12-19 MediaElch used TravisCI.  They heavily reduced their free
plan so that we were forced to drop TravisCI and switch to another CI runner.
TravisCI was used for uploading nightlies, uploading code coverage and more.  For historic
reasons, you can find the `.travis.yml` file [here][travis_old].

Before that a private GitLab CI instance was used to deploy a nightly version of MediaElch.
You can find the deprecated CI script [here][GitLabKomet].

Since 2021-02-10 we use a private Jenkins instance.  


## What does Jenkins do?

The [`Jenkinsfile`](../../Jenkinsfile) file contains all steps that are run each time
a pull request is opened or a change is committed.  These include:

 1. Checks
    - `clang-format` for C++ formatting
    - `cmake-format` for CMake formatting
    - `cppcheck` for common C++ issues and bugs
    - `shellcheck` for linting Bash scripts
 2. Builds
    - TODO: macOS build (latest Qt version using Homebrew)
    - TODO: Ubuntu builds (oldest still supported Qt version + latest Qt version)
    - TODO: Windows builds (using `mxe`; builds Windows executables on Linux)
 3. Tests
    - Build with latest LTS Qt version and latest Ubuntu version on TravisCI
    - run unit and integration tests
 4. Deployment
    - TODO: successful builds are deployed to our binary provider


[bintray]: https://bintray.com/komet/MediaElch
[travis]: https://travis-ci.org/Komet/MediaElch
[GitlabKomet]: https://github.com/Komet/MediaElch/blob/fafd391f83325a52a3e101684b88c7622cb085a0/.gitlab-ci.yml
[wikipedia_ci]: (https://en.wikipedia.org/wiki/Continuous_integration)
[travis_old]: https://github.com/Komet/MediaElch/blob/97bde70523ce7627a9c34503e45478b449ce93b0/.travis.yml
