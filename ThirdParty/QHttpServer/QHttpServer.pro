QT += network
QT -= gui

TEMPLATE = lib
TARGET = QHttpServer

DEFINES += QHTTPSERVER_LIBRARY

HEADERS =  http-parser/http_parser.h \
           QHttpConnection.h \
           QHttpServer.h \
           QHttpRequest.h \
           QHttpResponse.h \
           QHttpServerApi.h \
           QHttpServerFwd.h

SOURCES =   http-parser/http_parser.c \
            QHttpConnection.cpp \
            QHttpRequest.cpp \
            QHttpResponse.cpp \
            QHttpServer.cpp

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}


