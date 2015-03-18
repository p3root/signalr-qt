#-------------------------------------------------
#
# Project created by QtCreator 2014-03-31T15:26:40
#
#-------------------------------------------------

QT       += network testlib

QT       -= gui

TARGET = tst_unitteststest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    HttpEventStreamParserTest.cpp \
    main.cpp


HEADERS += \
    HttpEventStreamParserTest.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"

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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QHttpServer/src/release/ -lQHttpServer
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QHttpServer/src/debug/ -lQHttpServer
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QHttpServer/src/ -lQHttpServer

INCLUDEPATH += $$PWD/../../ThirdParty/QHttpServer/src
DEPENDPATH += $$PWD/../../ThirdParty/QHttpServer/src

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/release/ -lQextJson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/debug/ -lQextJson
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../../ThirdParty/QtExtJson

