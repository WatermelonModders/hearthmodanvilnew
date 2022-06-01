#-------------------------------------------------
#
# Project created by QtCreator 2016-06-13T19:13:31
#
#-------------------------------------------------

QT       += core gui network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hearthmod
TEMPLATE = app

SOURCES += main.cpp\
    proto.cpp \
    mainwindow.cpp

HEADERS  += \
    proto.h \
    mainwindow.h

FORMS    += \
    mainwindow.ui

DISTFILES += \
    wait.cur
