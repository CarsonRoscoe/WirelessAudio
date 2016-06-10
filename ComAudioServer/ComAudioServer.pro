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
    readfileworker.cpp \
    servercontrolchannel.cpp \
    serverreceive.cpp \
    serversend.cpp \
    serverudp.cpp \
    ../ComAudioClient/circularbuffer.cpp

HEADERS  += mainwindow.h \
    readfileworker.h \
    serverudp.h \
    ../ComAudioClient/circularbuffer.h


HEADERS  += mainwindow.h \
    server.h \

FORMS    += mainwindow.ui

DISTFILES += \
    ComAudioServer.pro.user

RESOURCES += \
    ../ComAudioClient/sweetjesus.qrc
