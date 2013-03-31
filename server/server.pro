#-------------------------------------------------
#
# Project created by QtCreator 2011-09-08T10:38:45
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = QTChatServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    server.cpp \
    chatclient.cpp

HEADERS += \
    server.h \
    chatclient.h
