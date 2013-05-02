#-------------------------------------------------
#
# Project created by QtCreator 2013-05-02T21:47:47
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = SignalRServerSample
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../SignalR/SignalRCore/release/ -lSignalRCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../SignalR/SignalRCore/debug/ -lSignalRCore
else:symbian: LIBS += -lSignalRCore
else:unix: LIBS += -L$$OUT_PWD/../../SignalR/SignalRCore/ -lSignalRCore

INCLUDEPATH += $$PWD/../../SignalR/SignalRCore
DEPENDPATH += $$PWD/../../SignalR/SignalRCore
