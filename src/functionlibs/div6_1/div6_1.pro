
QT += core gui

VERSION = 1.0.0

TARGET = div6_1

TEMPLATE = lib

SOURCES += \
    div6_1.cpp \
    dllmain.cpp \
    ../../common/guid.cpp

HEADERS += \
    div6_1_global.h \
    div6_1.h \
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
