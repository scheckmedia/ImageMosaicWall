#!/bin/bash

set -x
set -e

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}
trap cleanup EXIT

REPO_ROOT=$(readlink -f $(dirname $(dirname $0)))
mkdir -p $REPO_ROOT/releases
mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

cmake $REPO_ROOT -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
make install DESTDIR=AppDir


mkdir -p ${BUILD_DIR}/AppDir/usr/share/icons/hicolor/256x256 ${BUILD_DIR}/AppDir/usr/share/applications
cp ${REPO_ROOT}/assets/ImageMosaicWall.desktop ${BUILD_DIR}/AppDir/usr/share/applications/
cp ${REPO_ROOT}/assets/imw-logo.png ${BUILD_DIR}/AppDir/usr/share/icons/hicolor/256x256/ImageMosaicWall.png

wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage -P ${BUILD_DIR}
chmod a+x ${BUILD_DIR}/linuxdeployqt-*.AppImage

./linuxdeployqt-continuous-x86_64.AppImage AppDir/usr/share/applications/*.desktop -appimage

cp ImageMosaicWall*.AppImage $REPO_ROOT/releases/
