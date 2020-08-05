#include <QApplication>
#include <QFontDatabase>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Q_INIT_RESOURCE(assets);

    if (QFontDatabase::addApplicationFont(":/fonts/assets/SourceSansPro-Regular.ttf") < 0)
        qWarning() << "SourceSansPro-Regular cannot be loaded !";

    MainWindow w;
    w.setWindowState(Qt::WindowMaximized);
    w.show();

    return a.exec();
}
