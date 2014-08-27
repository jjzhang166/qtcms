QT += core xml webkit

TEMPLATE = lib
TARGET = Settingsactivity

VERSION = 1.1.12.0

SOURCES += dllmain.cpp \
    SettingsactivityThread.cpp \
    settingsactivity.cpp \
    ../../common/guid.cpp

HEADERS += SettingsactivityThread.h \
    settingsactivity.h \
    settingsactivity_global.h \
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
