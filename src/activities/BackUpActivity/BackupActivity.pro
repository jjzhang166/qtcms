TARGET=BackUpActivity
VERSION=1.1.12.0
TEMPLATE=lib
QT += core webkit

SOURCES += dllmain.cpp \
    backupactivity.cpp \
    ../../common/guid.cpp

HEADERS += dllmain.h \
    backupactivity.h \
    backupactivity_global.h \
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
    DESTDIR = ../../../mac/release/activities
}

LIBS += -lpcom
