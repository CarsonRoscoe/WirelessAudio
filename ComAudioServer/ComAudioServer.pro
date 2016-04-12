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
    serverreceive.cpp \
    serversend.cpp \
    ../ComAudioClient/circularbuffer.cpp \
    servercontrolchannel.cpp

HEADERS  += mainwindow.h \
    server.h \
    ../ComAudioClient/circularbuffer.h

FORMS    += mainwindow.ui

DISTFILES += \
    ComAudioServer.pro.user
