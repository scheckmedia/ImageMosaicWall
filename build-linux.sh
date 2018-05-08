mkdir -p ${builddir}/usr/share/icons/hicolor/256x256 ${builddir}/usr/share/applications ${builddir}/usr/lib ${builddir}/usr/bin
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage -P ${builddir}
chmod a+x ${builddir}/linuxdeployqt-*.AppImage
cp assets/ImageMosaicWall.desktop ${builddir}/usr/share/applications/
cp assets/imw-logo.png ${builddir}/usr/share/icons/hicolor/256x256/ImageMosaicWall.png
cp ${builddir}/ImageMosaicWall ${builddir}/usr/bin/
cd ${builddir}
./linuxdeployqt-continuous-x86_64.AppImage usr/share/applications/*.desktop -bundle-non-qt-libs
./linuxdeployqt-continuous-x86_64.AppImage usr/share/applications/*.desktop -appimage
