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
    thread.cpp

HEADERS  += mainwindow.hpp \
    dbmanager.hpp \
    wikiinfo.hpp \
    thread.hpp

RESOURCES += \
    ressources.qrc
