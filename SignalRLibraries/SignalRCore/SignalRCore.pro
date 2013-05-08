#-------------------------------------------------
#
# Project created by QtCreator 2013-05-02T21:29:01
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = SignalRCore
TEMPLATE = lib

DEFINES += SIGNALRCORE_LIBRARY

SOURCES += \
    ConnectionListener.cpp \
    ConnectionManager.cpp \
    SignalRServer.cpp \
    ConnectionHelper.cpp \
    Messages/NegotiationMessage.cpp \
    Transports/Transport.cpp \
	Http/HttpRequest.cpp \
	Http/HttpResponse.cpp

HEADERS +=\
    SignalRCore_global.h \
    ConnectionListener.h \
    ConnectionManager.h \
    SignalRServer.h \
    ConnectionHelper.h \
    Messages/NegotiationMessage.h \
    Transports/Transport.h \
	Http/HttpRequest.h \
	Http/HttpResponse.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/release/ -lQextJson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/debug/ -lQextJson
else:symbian: LIBS += -lQextJson
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../../ThirdParty/QtExtJson

