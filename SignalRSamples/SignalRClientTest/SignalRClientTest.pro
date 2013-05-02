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
           MyConnectionHandler.cpp \
    Client.cpp

HEADERS += MyConnectionHandler.h \
    Client.h


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtLog/release/ -lQtExtLog
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtLog/debug/ -lQtExtLog
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtLog/ -lQtExtLog

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtLog
DEPENDPATH += $$PWD/../../ThirdParty/QtExtLog

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/release/ -lQextJson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../T../hirdParty/QtExtJson/debug/ -lQextJson
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../../ThirdParty/QtExtJson

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../SignalRClient/release/ -lSignalRClient
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../SignalRClient/debug/ -lSignalRClient
else:unix: LIBS += -L$$OUT_PWD/../../SignalRClient/ -lSignalRClient

INCLUDEPATH += $$PWD/../../SignalRClient
DEPENDPATH += $$PWD/../../SignalRClient
