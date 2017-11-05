#-------------------------------------------------
#
# Project created by QtCreator 2013-04-25T20:33:46
#
#-------------------------------------------------

QT += network

QT -= gui

CONFIG   += precompile_header

TARGET = SignalRClient
TEMPLATE = lib


DEFINES += SIGNALR_LIBRARY

SOURCES += \
        Transports/LongPollingTransport.cpp\
        Transports/HttpBasedTransport.cpp\
        Transports/ServerSentEventsTransport.cpp\
        Transports/HttpEventStream.cpp \
        ClientTransport.cpp \
        Connection.cpp \
        SignalException.cpp \
        Helper/Helper.cpp \
        Helper/TransportHelper.cpp \
        Hubs/HubConnection.cpp \
        Hubs/HubProxy.cpp \
        Transports/HttpClient.cpp \
        Hubs/HubCallback.cpp \
        KeepAliveData.cpp \
        HeartbeatMonitor.cpp \
        Transports/WebSocketTransport.cpp \
        Transports/AutoTransport.cpp \
        Connection_p.cpp \
    Transports/HttpEventStreamParser.cpp

PUBLIC_HEADERS +=  \
        Transports/HttpBasedTransport.h \
        Transports/LongPollingTransport.h \
        Transports/ServerSentEventsTransport.h \
        Connection.h \
        ClientTransport.h \
        SignalException.h \
        Hubs/HubConnection.h \
        Hubs/HubProxy.h \
        Hubs/HubCallback.h \
        HeartbeatMonitor.h \
        Transports/WebSocketTransport.h \
        Transports/AutoTransport.h \
        SignalR.h \
        SignalR_global.h \
        Transports/NegotiateResponse.h \

PRIVATE_HEADERS +=  \
        Transports/HttpEventStream.h \
        Helper/TransportHelper.h \
        Helper/Helper.h \
        Transports/HttpClient.h \
        KeepAliveData.h \
        Connection_p.h \
        Transports/HttpEventStreamParser.h

HEADERS += $$PUBLIC_HEADERS \
           $$PRIVATE_HEADERS \
    Hubs/Argument.h



target.path = $$[QT_INSTALL_PREFIX]/lib
headers.files = $$PUBLIC_HEADERS
headers.path = $$[QT_INSTALL_PREFIX]/include/signalr

INSTALLS += target headers


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/release/ -lQextJson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/debug/ -lQextJson
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../../ThirdParty/QtExtJson

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtWebSockets/src/websockets/release/ -lQtWebSockets
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QtWebSockets/src/websockets/debug/ -lQtWebSockets
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtWebSockets/src/websockets/ -lQtWebSockets

INCLUDEPATH += $$PWD/../../ThirdParty/QtWebSockets/src/websockets
DEPENDPATH += $$PWD/../../ThirdParty/QtWebSockets/src/websockets
