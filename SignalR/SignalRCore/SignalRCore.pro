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
    SignalRServer.cpp

HEADERS +=\
        SignalRCore_global.h \
    ConnectionListener.h \
    ConnectionManager.h \
    SignalRServer.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE33AD1A1
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = SignalRCore.dll
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
