#-------------------------------------------------
#
# Project created by QtCreator 2013-07-10T17:44:05
#
#-------------------------------------------------

QT       += core gui xml
QT       += webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QTTest
TEMPLATE = app

debug{
    LIBS += -L../../mac/debug
}else{
    LIBS += -L../../mac/release
}

VERSION=1.1.12.0

LIBS += -lpcom

SOURCES += main.cpp \
    qjawebview.cpp \
    ../common/guid.cpp\
    webkitpluginsfactory.cpp \
    MyEventSender.cpp \

HEADERS  += \
    qjawebview.h \
    webkitpluginsfactory.h \
    MyEventSender.cpp

FORMS    +=

debug{
    OBJECTS_DIR = ./objs/Debug
    MOC_DIR = ./moc/Debug
    DESTDIR = ../../mac/debug
}else{
    OBJECTS_DIR = ./objs/Release
    MOC_DIR = ./moc/Release
    DESTDIR = ../../mac/release
}

INCLUDEPATH += ../../include




