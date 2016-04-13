#-------------------------------------------------
#
# Project created by QtCreator 2016-04-07T01:21:05
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ComAudioClient
TEMPLATE = app
LIBS = -lws2_32

SOURCES += main.cpp\
        mainwindow.cpp \
    audiomanager.cpp \
    circularbuffer.cpp \
    ClientReceive.cpp \
    ClientSend.cpp \
    clientudp.cpp \
    populatebufferworker.cpp \
    populatemicrophoneworker.cpp \
    readfileworker.cpp \
    udpthread.cpp


HEADERS  += mainwindow.h \
    audiomanager.h \
    circularbuffer.h \
    Client.h \
    clientudp.h \
    populatebufferworker.h \
    populatemicrophoneworker.h \
    readfileworker.h \
    songstate.h \
    udpthread.h \
    wavheader.h


FORMS    += mainwindow.ui

DISTFILES += \
    ComAudioClient.pro.user
