QT += core webkit

VERSION=1.1.12.0
TARGET=PlaybackActivity
TEMPLATE=lib

SOURCES += dllmain.cpp \
    playbackactivity.cpp \
    ../../common/guid.cpp

HEADERS += dllmain.h \
    playbackactivity.h \
    playbackactivity_global.h \
    ../../../include/qwfw.h

INCLUDEPATH += ../../../include

debug{
    LIBS += -L../../../mac/debug
    OBJECTS_DIR = ./objs/debug
    MOC_DIR = ./moc/debug
    DESTDIR = ../../../mac/debug/activities
} else {
    LIBS += -L../../../mac/release
    OBJECTS_DIR = ./objs/release
    MOC_DIR = ./moc/release
    DESTDIR = ../../../mac/debug/activities
}

LIBS += -lpcom
