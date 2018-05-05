#-------------------------------------------------
#
# Project created by QtCreator 2018-04-25T16:32:28
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageMosaicWall
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



CONFIG += c++11
SOURCES += \
        main.cpp \
        mainwindow.cpp \
    imageviewer.cpp \
    imageprocessing.cpp


HEADERS += \
        mainwindow.h \
    imageviewer.h \
    imageprocessing.h


FORMS += \
        mainwindow.ui \
    imageviewer.ui

RESOURCES += \
    assets.qrc

DISTFILES += \
    default.qss

macx: {
  # I dont know why the hack this lines not working.. so then we get the ugly absolute path
  INCLUDEPATH += /usr/local/include/
  DEPENDPATH += /usr/local/include/
  LIBS += -L /usr/local/lib/ -lexiv2  
}

linux: {
  INCLUDEPATH += /usr/include/
  DEPENDPATH += /usr/include/
  LIBS += -L /usr/lib/x86_64-linux-gnu/ -lexiv2
}
