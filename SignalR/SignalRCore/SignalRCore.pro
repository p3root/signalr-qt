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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../SignalRHttp/release/ -lSignalRHttp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../SignalRHttp/debug/ -lSignalRHttp
else:symbian: LIBS += -lSignalRHttp
else:unix: LIBS += -L$$OUT_PWD/../SignalRHttp/ -lSignalRHttp

INCLUDEPATH += $$PWD/../SignalRHttp
DEPENDPATH += $$PWD/../SignalRHttp
