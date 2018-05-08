mkdir -p ${TRAVIS_BUILD_DIR-build}/usr/share/icons/hicolor/256x256 ${TRAVIS_BUILD_DIR-build}/usr/share/applications ${TRAVIS_BUILD_DIR-build}/usr/lib ${TRAVIS_BUILD_DIR-build}/usr/bin
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage -P ${TRAVIS_BUILD_DIR-build}
chmod a+x ${TRAVIS_BUILD_DIR-build}/linuxdeployqt-*.AppImage
${TRAVIS_BUILD_DIR-build}/linuxdeployqt-*.AppImage --appimage-extract
echo "[Desktop Entry]\n
  Type=Application\n
  Name=Image Mosaic Wall\n
  Comment=Create a Mosaic Image Wall from a set of images\n
  Exec=ImageMosaicWall\n
  Icon=ImageMosaicWall\n
  Categories=Office;\n" >> ${TRAVIS_BUILD_DIR-build}/usr/share/applications/ImageMosaicWall.desktop

cp ${TRAVIS_BUILD_DIR}/assets/imw-logo.png ${TRAVIS_BUILD_DIR-build}/usr/share/icons/hicolor/256x256/ImageMosaicWall.png
cp ${TRAVIS_BUILD_DIR-build}/ImageMosaicWall ${TRAVIS_BUILD_DIR-build}/usr/bin/
${TRAVIS_BUILD_DIR-build}/linuxdeployqt-continuous-x86_64.AppImage ${TRAVIS_BUILD_DIR-build}/usr/share/applications/*.desktop -bundle-non-qt-libs
${TRAVIS_BUILD_DIR-build}/linuxdeployqt-continuous-x86_64.AppImage ${TRAVIS_BUILD_DIR-build}/usr/share/applications/*.desktop -appimage
