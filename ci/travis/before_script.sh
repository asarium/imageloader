#!/usr/bin/env sh

set -e

mkdir -p travis-build
cd travis-build

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    $HOME/cmake/bin/cmake -G "Ninja" -DCMAKE_BUILD_TYPE=$CONFIGURATION ..
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    $HOME/cmake/CMake.app/Contents/bin/cmake -G "Xcode" ..
fi
