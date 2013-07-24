#-------------------------------------------------
#
# Project created by QtCreator 2013-07-24T20:44:23
#
#-------------------------------------------------

QT       += xml

QT       -= gui

TARGET = libpcom
TEMPLATE = lib

DEFINES += LIBPCOM_LIBRARY

SOURCES += libpcom.cpp

HEADERS += libpcom.h\
        libpcom_global.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
