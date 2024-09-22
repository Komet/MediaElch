#!/usr/bin/env bash

###########################################################
# Create all docker images and build MediaElch
###########################################################

set -euo pipefail
IFS=$'\n\t'

cd "$(dirname "$0")"

./docker-build-dist.sh ubuntu-16.04        | tee -a docker-build.log
./docker-build-dist.sh ubuntu-20.04        | tee -a docker-build.log
./docker-build-dist.sh ubuntu-22.04        | tee -a docker-build.log
./docker-build-dist.sh ubuntu-24.04        | tee -a docker-build.log
./docker-build-dist.sh opensuse-leap-15    | tee -a docker-build.log
./docker-build-dist.sh opensuse-tumbleweed | tee -a docker-build.log
