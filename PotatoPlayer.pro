#-------------------------------------------------
#
# Project created by QtCreator 2013-01-04T23:20:22
#
#-------------------------------------------------

QT       += core gui phonon sql webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PotatoPlayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dbmanager.cpp \
    wikiinfo.cpp \
    scandir.cpp \
    insertpl.cpp \
    tageditor.cpp

HEADERS  += mainwindow.hpp \
    dbmanager.hpp \
    wikiinfo.hpp \
    scandir.hpp \
    insertpl.hpp \
    tageditor.hpp

RESOURCES += \
    ressources.qrc

TRANSLATIONS += \
    potato_en.ts
