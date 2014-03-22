
QT += core gui

VERSION = 1.0.0

TARGET = div7_7

TEMPLATE = lib

SOURCES += \
    div7_7.cpp \
    dllmain.cpp \
    ../../common/guid.cpp

HEADERS += \
    div7_7_global.h \
    div7_7.h \
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
