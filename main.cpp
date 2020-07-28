#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  Q_INIT_RESOURCE(assets);

  if (QFontDatabase::addApplicationFont(
          ":/fonts/assets/MaterialIcons-Regular.ttf") < 0)
    qWarning() << "MaterialIcons cannot be loaded !";

  if (QFontDatabase::addApplicationFont(
          ":/fonts/assets/SourceSansPro-Regular.ttf") < 0)
    qWarning() << "SourceSansPro-Regular cannot be loaded !";

  MainWindow w;
  w.setWindowState(Qt::WindowMaximized);
  w.show();

  return a.exec();
}
