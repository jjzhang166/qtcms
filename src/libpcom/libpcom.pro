#-------------------------------------------------
#
# Project created by QtCreator 2013-07-24T20:44:23
#
#-------------------------------------------------

QT       += xml

QT       -= gui

VERSION = 1.0.0

TARGET = pcom
TEMPLATE = lib

DEFINES += LIBPCOM_LIBRARY

SOURCES += libpcom.cpp

HEADERS += libpcom.h

INCLUDEPATH += ../../include

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

debug {
    OBJECTS_DIR = ./objs/Debug
    MOC_DIR = ./moc/Debug
    DESTDIR = ../../mac/debug
} else {
    OBJECTS_DIR = ./objs/Release
    MOC_DIR = ./moc/Release
    DESTDIR = ../../mac/release
}

OTHER_FILES += \
    pcom_config.xml



