VERSION=1.1.12.0
TARGET=HiChipSearch
TEMPLATE=lib

QT += core network

SOURCES += dllmain.cpp \
    HiChipSearch.cpp \
    ../../common/guid.cpp

HEADERS += dllmain.h \
    HiChipSearch_global.h \
    HiChipSearch.h

INCLUDEPATH += ../../../include

debug {
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
