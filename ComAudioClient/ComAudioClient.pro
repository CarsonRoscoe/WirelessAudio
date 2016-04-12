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
    populatebufferworker.cpp \
    readfileworker.cpp \
    clientcontrolchannel.cpp

HEADERS  += mainwindow.h \
    audiomanager.h \
    circularbuffer.h \
    Client.h \
    populatebufferworker.h \
    readfileworker.h \
    songstate.h \
    wavheader.h

FORMS    += mainwindow.ui

DISTFILES += \
    ComAudioClient.pro.user
