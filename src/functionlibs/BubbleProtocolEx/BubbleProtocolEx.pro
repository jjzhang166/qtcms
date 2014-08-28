TARGET=BubbleProtocolEx
VERSION=1.1.12.0
TEMPLATE=lib

QT += core network xml

SOURCES += SearchRemoteFile.cpp \
    bubbleprotocolex.cpp \
    dllmain.cpp \
    h264wh.cpp \
    ../../common/guid.cpp

HEADERS += SearchRemoteFile.h \
    bubbleprotocolex.h \
    bubbleprotocolex_global.h \
    dllmain.h \
    h264wh.h

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
