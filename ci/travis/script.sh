#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    cd travis-build
    ninja all
    ./test/imgload_test
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cmake --build travis-build --target all --config $CONFIGURATION | xcpretty -c
    ./travis-build/test/$CONFIGURATION/imgload_test
fi


