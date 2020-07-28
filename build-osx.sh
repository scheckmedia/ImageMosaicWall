#!/bin/bash

set -x
set -e

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}
trap cleanup EXIT

mkdir -p $TRAVIS_BUILD_DIR/releases
mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

cmake $TRAVIS_BUILD_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
make
macdeployqt ImageMosaicWall.app -dmg
cp ImageMosaicWall*.dmg $TRAVIS_BUILD_DIR/releases/
