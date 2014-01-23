#-------------------------------------------------
#
# Project created by QtCreator 2014-01-16T13:19:42
#
#-------------------------------------------------

QT       += core network
QT       -= gui

TARGET = SignalRServerTest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../SignalRLibraries/SignalRServer/release/ -lSignalRServer
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../SignalRLibraries/SignalRServer/debug/ -lSignalRServer
else:unix: LIBS += -L$$OUT_PWD/../../SignalRLibraries/SignalRServer/ -lSignalRServer

INCLUDEPATH += $$PWD/../../SignalRLibraries/SignalRServer
DEPENDPATH += $$PWD/../../SignalRLibraries/SignalRServer

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/release/ -lQextJson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/debug/ -lQextJson
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../../ThirdParty/QtExtJson

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/qhttpserver/src/release/ -lqhttpserver
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/qhttpserver/src/debug/ -lqhttpserver
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/qhttpserver/src/ -lqhttpserver

INCLUDEPATH += $$PWD/../../ThirdParty/qhttpserver/src
DEPENDPATH += $$PWD/../../ThirdParty/qhttpserver/src
