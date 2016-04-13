#-------------------------------------------------
#
# Project created by QtCreator 2016-04-07T01:16:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ComAudioServer
TEMPLATE = app
LIBS = -lws2_32

SOURCES += main.cpp\
        mainwindow.cpp \
    circularbuffer.cpp \
    serverreceive.cpp \
    serversend.cpp \
    serverudp.cpp \
    readfileworker.cpp

HEADERS  += mainwindow.h \
    circularbuffer.h \
    server.h \
    serverudp.h \
    readfileworker.h

FORMS    += mainwindow.ui

DISTFILES += \
    ComAudioServer.pro.user
