#-------------------------------------------------
#
# Project created by QtCreator 2014-01-16T10:42:23
#
#-------------------------------------------------

QT       += core network
QT       -= gui

TARGET = SignalRServer
TEMPLATE = lib

DEFINES += SIGNALRSERVER_LIBRARY

SOURCES += SignalRServer.cpp \
    PersistentConnection.cpp \
    Infrastructure/EmptyProtectionData.cpp \
    Configuration/ConfigurationManager.cpp \
    Infrastructure/Purpose.cpp \
    Transport/TransportManager.cpp \
    Transport/LongPollingTransport.cpp \
    Infrastructure/ServerRequest.cpp \
    Infrastructure/Principal.cpp \
    Infrastructure/ServerResponse.cpp \
    Infrastructure/HostContext.cpp \
    Infrastructure/PrefixHelper.cpp \
    Transport/TransportBase.cpp \
    Infrastructure/Connection.cpp \
    ConnectionMessage.cpp \
    Messaging/Message.cpp \
    Messaging/MessageResult.cpp \
    Messaging/MessageBus.cpp \
    Transport/TransportHeartBeat.cpp \
    Infrastructure/StringMinifier.cpp \
    Messaging/TopicLookup.cpp \
    Messaging/Topic.cpp \
    Messaging/Subscription.cpp

HEADERS += SignalRServer.h\
        signalrserver_global.h \
    PersistentConnection.h \
    Infrastructure/Purpose.h \
    Infrastructure/EmptyProtectionData.h \
    Infrastructure/IProtectedData.h \
    Configuration/ConfigurationManager.h \
    Transport/TransportManager.h \
    Transport/LongPollingTransport.h \
    Transport/TransportBase.h \
    Infrastructure/ServerRequest.h \
    Infrastructure/Principal.h \
    Infrastructure/ServerResponse.h \
    Infrastructure/HostContext.h \
    Infrastructure/PrefixHelper.h \
    Infrastructure/Connection.h \
    IConnection.h \
    ConnectionMessage.h \
    Messaging/IMessageBus.h \
    Messaging/Message.h \
    Messaging/MessageResult.h \
    Messaging/MessageBus.h \
    Transport/TransportHeartBeat.h \
    Infrastructure/IStringMinifier.h \
    Infrastructure/StringMinifier.h \
    Messaging/TopicLookup.h \
    Messaging/Topic.h \
    Messaging/Subscription.h \
    Messaging/MessageStore.hpp

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
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QtExtJson/ -lQextJson

INCLUDEPATH += $$PWD/../../ThirdParty/QtExtJson
DEPENDPATH += $$PWD/../../ThirdParty/QtExtJson


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QHttpServer/src/release/ -lQHttpServer
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ThirdParty/QHttpServer/src/debug/ -lQHttpServer
else:unix: LIBS += -L$$OUT_PWD/../../ThirdParty/QHttpServer/src/ -lQHttpServer

INCLUDEPATH += $$PWD/../../ThirdParty/QHttpServer/src
DEPENDPATH += $$PWD/../../ThirdParty/QHttpServer/src
