#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    cd travis-build
    ninja dds_tests
    ./test/dds_tests
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cmake --build travis-build --target dds_tests --config $CONFIGURATION | xcpretty -c
    ./travis-build/test/$CONFIGURATION/dds_tests
fi


