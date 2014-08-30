TEMPLATE=lib
VERSION=1.1.12.0
TARGET=DeviceClient

QT += core

SOURCES += BufferManager.cpp \
    GlobalSettings.cpp \
    PlayManager.cpp \
    RemoteBackup.cpp \
    avilib.cpp \
    deviceclient.cpp \
    dllmain.cpp \
    remotePlayBack.cpp \
    ../../common/guid.cpp

HEADERS += BufferManager.h \
    GlobalSettings.h \
    RemoteBackup.h \
    deviceclient.h \
    dllmain.h \
    remotePlayBack.h \
    DeviceGlobalSettings.h \
    PlayManager.h \
    avilib.h \
    deviceclient_global.h \
    netlib.h

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
