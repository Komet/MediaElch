#!/usr/bin/env bash

set -e

cd ~
mkdir temp && cd $_
wget https://cmake.org/files/v3.18/cmake-3.18.3-Linux-x86_64.sh
mkdir /opt/cmake
sh cmake-3.18.3-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
cd ..
rm -rf temp
