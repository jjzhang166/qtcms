#-------------------------------------------------
#
# Project created by QtCreator 2013-07-10T17:44:05
#
#-------------------------------------------------

QT       += core gui
QT       += webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QTTest
TEMPLATE = app

LIBS += ./libpcom.lib

SOURCES += main.cpp \
    qjawebview.cpp

HEADERS  += \
    qjawebview.h

FORMS    +=

INCLUDEPATH += ../include

