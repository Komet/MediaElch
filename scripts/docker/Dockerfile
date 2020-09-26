FROM ubuntu:latest

LABEL maintainer="info@andremeyering.de"

VOLUME ["/opt/mediaelch"]

#
# This image can be used to develop and test MediaElch
#

# Developer Tools

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y software-properties-common && \
    add-apt-repository ppa:ubuntu-toolchain-r/test

RUN apt-get update && \
    apt-get install -y zlib1g zlib1g-dev g++-10 gcc-10 git wget curl \
        libclang-10-dev clang-tidy-10 clang-tools-10 clang-format-10 clang-10 \
        llvm-10-dev git-core xvfb \
        python3 python3-yaml python3-pip libjson-perl \
        libncurses5-dev libncurses5 ninja-build doxygen && \
    update-alternatives --install /usr/bin/gcc           gcc          /usr/bin/gcc-10          10 && \
    update-alternatives --install /usr/bin/gcov          gcov         /usr/bin/gcov-10         10 && \
    update-alternatives --install /usr/bin/g++           g++          /usr/bin/g++-10          10 && \
    update-alternatives --install /usr/bin/clang++       clang++      /usr/bin/clang++-10      10 && \
    update-alternatives --install /usr/bin/clang         clang        /usr/bin/clang-10        10 && \
    update-alternatives --install /usr/bin/clang-format  clang-format /usr/bin/clang-format-10 10 && \
    update-alternatives --install /usr/bin/clang-tidy    clang-tidy   /usr/bin/clang-tidy-10   10 && \
    update-alternatives --install /usr/bin/llvm-config   llvm-config  /usr/bin/llvm-config-10  10 && \
    update-alternatives --install /usr/bin/llvm-cov      llvm-cov     /usr/bin/llvm-cov-10     10

# Latest versions that aren't available through apt or pip
COPY install_cmake.sh    /opt/install_cmake.sh
COPY install_cppcheck.sh /opt/install_cppcheck.sh
COPY install_iwyu.sh     /opt/install_iwyu.sh

RUN /opt/install_cmake.sh
RUN /opt/install_cppcheck.sh
RUN /opt/install_iwyu.sh

# Latest lcov due to GCC9 issues with lcov 1.13
RUN cd /opt && git clone https://github.com/linux-test-project/lcov.git && cd lcov && make install
RUN perl -MCPAN -e 'install PerlIO::gzip'
RUN perl -MCPAN -e 'JSON'

RUN pip3 install cmake_format

# MediaElch dependencies
RUN apt-get -y --no-install-recommends install \
        libmediainfo-dev \
        ffmpeg \
        qt5-default \
        qtmultimedia5-dev \
        qtdeclarative5-dev \
        libqt5opengl5 \
        libqt5opengl5-dev && \
    apt-get autoremove

RUN cd /opt && git clone https://github.com/KDE/clazy.git
RUN cd /opt/clazy && git checkout 1.6

# Multicore build always fails for some reason, so we use -j1
RUN cd /opt/clazy && mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j1 && \
    make install
