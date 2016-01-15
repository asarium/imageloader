#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    cd travis-build
    ninja all
    valgrind --leak-check=full --error-exitcode=1 --gen-suppressions=all \
        --suppressions="$TRAVIS_BUILD_DIR/ci/travis/valgrind.supp" ./test/imgload_test
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cmake --build travis-build --config $CONFIGURATION | xcpretty -c
    ./travis-build/test/$CONFIGURATION/imgload_test
fi


