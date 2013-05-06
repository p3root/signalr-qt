#-------------------------------------------------
#
# Project created by QtCreator 2013-05-02T22:17:45
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = SignalRHttp
TEMPLATE = lib

DEFINES += SIGNALRHTTP_LIBRARY

SOURCES += HttpRequest.cpp \
    HttpResponse.cpp

HEADERS += HttpRequest.h\
        SignalRHttp_global.h \
    HttpResponse.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE3236157
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = SignalRHttp.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
