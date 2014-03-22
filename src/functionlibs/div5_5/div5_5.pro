
QT += core gui

VERSION = 1.0.0

TARGET = div5_5

TEMPLATE = lib

SOURCES += \
    div5_5.cpp \
    dllmain.cpp \
    ../../common/guid.cpp

HEADERS += \
    div5_5_global.h \
    div5_5.h \
    dllmain.h

INCLUDEPATH += ../../../include

debug {
    LIBS += -L../../../mac/debug
} else {
    LIBS += -L../../../mac/release
}

LIBS += -lpcom

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
    DESTDIR = ../../../mac/debug
} else {
    OBJECTS_DIR = ./objs/Release
    MOC_DIR = ./moc/Release
    DESTDIR = ../../../mac/release
}
