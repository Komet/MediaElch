# Continuous Integration (CI) and Continuous Delivery/Deployment (CD)

## What is CI?
See [Wikipedia](https://en.wikipedia.org/wiki/Continuous_integration). For MediaElch this means that we have one main code branch (`master`) and each code change (i.e. Git commit) is build and tested automatically by a CI service.
Furthermore every pull request is tested and will only be merged if all tests succeed.


## What is CD?
The term is often used in conjuction with webservices. When code is changed, tests are run automatically and if they succeed then the application is deployed automatically to production systems (*Note:* There are many definitions of CD and this one is incomplete).

For MediaElch, this means that every code change to the `master` branch is build by one CI service and is then deployed to [Bintray][bintray] (see below). This way the latest change can always be tested by users and we don't have to deploy anything by hand.


## What CI services do you use?
Since 2018-03-10 we use [TravisCI][travis] as our primary continuous integration service (see TravisCI section below). Before that a private GitLab CI instance was used to deploy a nightly version of MediaElch. You can find the deprecated CI script [here][GitLabKomet].


## What does TravisCI do?
The [`.travis.yml`](https://github.com/Komet/MediaElch/blob/master/.travis.yml) file contains all steps that are run each time a pull request is opened or a change is commited. These include:

 1. Checks
    - `clang-format` for C++ formatting
    - `cmake-format` for CMake formatting
    - `cppcheck` for common C++ issues and bugs
    - `shellcheck` for linting Bash scripts
 2. Builds
    - macOS build (latest Qt version using Homebrew)
    - Ubuntu builds (oldest still supported Qt version + latest Qt version)
    - Windows builds (using `mxe`; builds Windows executables on Linux)
 3. Tests
    - Build with latest LTS Qt version and latest Ubuntu version on TravisCI
    - run unit and integration tests
 4. Deployment
    - successful builds are deployed to [Bintray][bintray]

## Other CI services
We may use GitLab CI or Jenkins in the future but at the moment no plans exists to do so.


[bintray]: https://bintray.com/komet/MediaElch
[travis]: https://travis-ci.org/Komet/MediaElch
[GitlabKomet]: https://github.com/Komet/MediaElch/blob/fafd391f83325a52a3e101684b88c7622cb085a0/.gitlab-ci.yml
