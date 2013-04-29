#-------------------------------------------------
#
# Project created by QtCreator 2013-04-25T21:30:14
#
#-------------------------------------------------

QT       += core
QT       += network

QT       -= gui

TARGET = SignalRClientTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
           MyConnectionHandler.cpp

HEADERS += MyConnectionHandler.h



unix:!macx:!symbian: LIBS += -L$$OUT_PWD/../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../ThirdParty/QtExtJson

unix:!macx:!symbian: LIBS += -L$$OUT_PWD/../ThirdParty/QtExtLog/ -lQtExtLog

INCLUDEPATH += $$PWD/../ThirdParty/QtExtLog
DEPENDPATH += $$PWD/../ThirdParty/QtExtLog


unix:!macx:!symbian: LIBS += -L$$OUT_PWD/../SignalRClient/ -lSignalRClient

INCLUDEPATH += $$PWD/../SignalRClient
DEPENDPATH += $$PWD/../SignalRClient
