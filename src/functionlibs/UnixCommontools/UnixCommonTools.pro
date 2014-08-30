#-------------------------------------------------
#
# Project created by QtCreator 2014-08-28T19:10:26
#
#-------------------------------------------------

QT       += core

TARGET = UnixCommontools
TEMPLATE = lib

VERSION = 1.0.0.0

DEFINES += UNIXCOMMONTOOLS_LIBRARY

SOURCES += ../../common/guid.cpp \
    unixcommontools.cpp \
    dllmain.cpp

HEADERS += ../../../include/IStorage.h \
    unixcommontools.h \
    unixcommontools_global.h \
    dllmain.h

INCLUDEPATH += ../../../include

debug{
    LIBS += -L../../../mac/debug
    OBJECTS_DIR = ./objs/debug
    MOC_DIR = ./moc/debug
    DESTDIR = ../../../mac/debug
} else {
    LIBS += -L../../../mac/release
    OBJECTS_DIR = ./objs/release
    MOC_DIR = ./moc/release
    DESTDIR = ../../../mac/release
}

LIBS += -lpcom
