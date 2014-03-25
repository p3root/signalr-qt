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
    Client.cpp

HEADERS += \
    Client.h


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/release/ -lQextJson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/debug/ -lQextJson
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../../ThirdParty/QtExtJson

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../SignalRLibraries/SignalRClient/release/ -lSignalRClient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../SignalRLibraries/SignalRClient/debug/ -lSignalRClient
else:unix: LIBS += -L$$OUT_PWD/../../SignalRLibraries/SignalRClient/ -lSignalRClient

INCLUDEPATH += $$PWD/../../SignalRLibraries/SignalRClient
DEPENDPATH += $$PWD/../../SignalRLibraries/SignalRClient

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtWebSockets/src/websockets/release/ -lQtWebSockets
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtWebSockets/src/websockets/debug/ -lQtWebSockets
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtWebSockets/src/websockets/ -lQtWebSockets

INCLUDEPATH += $$PWD/../../ThirdParty/QtWebSockets/src/websockets
DEPENDPATH += $$PWD/../../ThirdParty/QtWebSockets/src/websockets
