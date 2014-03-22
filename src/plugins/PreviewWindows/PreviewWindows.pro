
QT += core gui webkit xml

greaterThan(QT_MAJOR_VERSION,4): QT += widgets

TEMPLATE = lib
TARGET = PreviewWindows

VERSION = 1.0.0

SOURCES += \
    dllmain.cpp \
    PreviewPlay.cpp \
    previewwindows.cpp \
    PreviewWindowsGlobalSetting.cpp \
    qpreviewwindows.cpp \
    qsubview.cpp \
    QSubViewObject.cpp \
    QSubviewThread.cpp \
    ../../common/guid.cpp

HEADERS += \
    dllmain.h \
    PreviewPlay.h \
    previewwindows_global.h \
    previewwindows.h \
    PreviewWindowsGlobalSetting.h \
    qpreviewwindows.h \
    qsubview.h \
    QSubViewObject.h \
    QSubviewThread.h

FORMS += \
    TitleView.ui

INCLUDEPATH += ../../../include

debug{
    LIBS += -L../../../mac/debug
    OBJECTS_DIR = ./objs/debug
    MOC_DIR = ./moc/debug
    DESTDIR = ../../../mac/debug/plugins
} else {
    LIBS += -L../../../mac/release
    OBJECTS_DIR = ./objs/release
    MOC_DIR = ./moc/release
    DESTDIR = ../../../mac/release/plugins
}

LIBS += -lpcom
