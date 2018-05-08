mkdir -p ${builddir}/usr/share/icons/hicolor/256x256 ${builddir}/usr/share/applications ${builddir}/usr/lib ${builddir}/usr/bin
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage -P ${builddir}
chmod a+x ${builddir}/linuxdeployqt-*.AppImage
echo "[Desktop Entry]\n
  Type=Application\n
  Name=Image Mosaic Wall\n
  Comment=Create a Mosaic Image Wall from a set of images\n
  Exec=ImageMosaicWall\n
  Icon=ImageMosaicWall\n
  Categories=Office;\n" >> ${builddir}/usr/share/applications/ImageMosaicWall.desktop

cp assets/imw-logo.png ${builddir}/usr/share/icons/hicolor/256x256/ImageMosaicWall.png
cp ${builddir}/ImageMosaicWall ${builddir}/usr/bin/
${builddir}/linuxdeployqt-continuous-x86_64.AppImage ${builddir}/usr/share/applications/*.desktop -bundle-non-qt-libs
${builddir}/linuxdeployqt-continuous-x86_64.AppImage ${builddir}/usr/share/applications/*.desktop -appimage
