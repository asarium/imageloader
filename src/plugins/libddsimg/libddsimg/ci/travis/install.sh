#!/usr/bin/env sh

set -e

FILENAME=
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    FILENAME=cmake-3.3.2-Linux-x86_64
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    FILENAME=cmake-3.3.2-Darwin-universal
    gem install xcpretty
fi

mkdir -p $HOME/cmake/

wget --no-check-certificate -O /tmp/cmake.tar.gz https://cmake.org/files/v3.3/$FILENAME.tar.gz
tar -xzf /tmp/cmake.tar.gz -C $HOME/cmake/ --strip-components=1
