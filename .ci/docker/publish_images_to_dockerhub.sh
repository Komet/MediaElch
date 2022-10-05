#!/usr/bin/env bash

###########################################################
# This script releases docker images to Docker Hub.
# These images are used for our CI pipelines, e.g. Jenkins.
#
# NOTE:  We now have Jenkins jobs that do this.
#        See ../jenkins/BuildDockerImages.groovy
###########################################################

set -euo pipefail
IFS=$'\n\t'

# Change to this directory
cd "$(dirname "$(realpath -s "$0")")"

docker build --pull -t mediaelch/mediaelch-ci-linux:latest -f Dockerfile.ci.linux .
docker push mediaelch/mediaelch-ci-linux:latest


docker build --pull -t mediaelch/mediaelch-ci-appimage:latest:latest -f Dockerfile.build-ubuntu-16.04 .
docker push mediaelch/mediaelch-ci-linux:latest


# Note: As of 2021-01-26, this may fail.
# mxe has an open issue for qtbase, see <https://github.com/mxe/mxe/issues/2590>
docker build --pull -t mediaelch/mediaelch-ci-win:latest -f Dockerfile.ci.windows .
docker push mediaelch/mediaelch-ci-win:latest
