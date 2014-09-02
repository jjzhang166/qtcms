VERSION=1.1.12.0
TARGET=DvrSearch
TEMPLATE=lib

QT += core network

SOURCES += dllmain.cpp \
    dvrsearch.cpp \
    ../../common/guid.cpp

HEADERS += dllmain.h \
    dvrsearch.h \
    dvrsearch_global.h \
    ../../../include/guid.h

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
