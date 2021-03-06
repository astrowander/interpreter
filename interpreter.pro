#-------------------------------------------------
#
# Project created by QtCreator 2015-11-28T09:34:53
#
#-------------------------------------------------
QMAKE_CXXFLAGS += -std=gnu++11

QT       += core

QT       -= gui

TARGET = interpreter
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    myvariant.cpp \
    expressions.cpp \
    statement.cpp \
    block.cpp \
    interpreter.cpp

HEADERS += \
    myvariant.h \
    expressions.h \
    statement.h \
    block.h \
    interpreter.h
