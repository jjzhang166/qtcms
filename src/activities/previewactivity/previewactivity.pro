QT += core gui network webkit

TEMPLATE = lib
TARGET = previewactivity

VERSION = 1.0.0

SOURCES += \
    dllmain.cpp \
    previewactivity.cpp \
    ../../common/guid.cpp

HEADERS += \
    dllmain.h \
    previewactivity_global.h \
    previewactivity.h \
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
