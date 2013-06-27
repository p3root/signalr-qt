#-------------------------------------------------
#
# Project created by QtCreator 2013-04-25T20:33:46
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = SignalRClient
TEMPLATE = lib

DEFINES += SIGNALR_LIBRARY

SOURCES +=  \
            Transports/HttpBasedTransport.cpp\
            Transports/LongPollingTransport.cpp\
            Transports/ServerSentEventsTransport.cpp\
            Transports/HttpResponse.cpp \
            Transports/HttpRequest.cpp \
            Transports/HttpEventStream.cpp \
            ClientTransport.cpp \
            Connection.cpp \
            SignalException.cpp \
            Helper/Helper.cpp \
            Helper/TransportHelper.cpp \
            Hubs/HubConnection.cpp \
            Hubs/HubProxy.cpp \
    Transports/HttpClient.cpp \
    Hubs/HubCallback.cpp



HEADERS +=  \
            Transports/HttpBasedTransport.h \
            Transports/LongPollingTransport.h \
            Transports/ServerSentEventsTransport.h \
            Transports/NegotiateResponse.h \
            Transports/HttpResponse.h \
            Transports/HttpRequest.h \
            Transports/HttpEventStream.h \
            Helper/TransportHelper.h \
            Helper/Helper.h \
            Connection.h \
            SignalR_global.h \
            ClientTransport.h \
            SignalException.h \
            Hubs/HubConnection.h \
            Hubs/HubProxy.h \
    Transports/HttpClient.h \
    Hubs/HubCallback.h



unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtLog/release/ -lQtExtLog
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtLog/debug/ -lQtExtLog
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtLog/ -lQtExtLog

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtLog
DEPENDPATH += $$PWD/../T../hirdParty/QtExtLog

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/release/ -lQextJson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/debug/ -lQextJson
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../../ThirdParty/QtExtJson
