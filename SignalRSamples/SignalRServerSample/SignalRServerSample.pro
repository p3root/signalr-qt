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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/release/ -lQextJson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/debug/ -lQextJson
else:symbian: LIBS += -lQextJson
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../../ThirdParty/QtExtJson

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../SignalRLibraries/SignalRCore/release/ -lSignalRCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../SignalRLibraries/SignalRCore/debug/ -lSignalRCore
else:symbian: LIBS += -lSignalRCore
else:unix: LIBS += -L$$OUT_PWD/../../SignalRLibraries/SignalRCore/ -lSignalRCore

INCLUDEPATH += $$PWD/../../SignalRLibraries/SignalRCore
DEPENDPATH += $$PWD/../../SignalRLibraries/SignalRCore
