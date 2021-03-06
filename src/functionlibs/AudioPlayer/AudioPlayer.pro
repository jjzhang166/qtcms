TARGET=AudioPlayer
VERSION=1.1.12.0
TEMPLATE=lib

QT = core

SOURCES += dllmain.cpp \
    AudioPlayer.cpp \
    ../../common/guid.cpp

HEADERS += dllmain.h \
    AudioPlayer.h \
    AudioPlayer_global.h

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
