#-------------------------------------------------
#
# Project created by QtCreator 2012-11-20T23:17:19
#
#-------------------------------------------------

QT       -= gui

TARGET = QtExtLog
TEMPLATE = lib

DEFINES += QTEXTLOG_LIBRARY

SOURCES +=  QsLogDest.cpp \
            QsLog.cpp \
            QsDebugOutput.cpp

HEADERS +=  QsLogDest.h \
            QsLog.h \
            QsDebugOutput.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEB5D4B6D
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = QtExtLog.dll
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
