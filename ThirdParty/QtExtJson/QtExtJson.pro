
QT       -= gui

TARGET = QextJson
TEMPLATE = lib
CONFIG += sharedlib


SOURCES += \
    QextJson.cpp

HEADERS += \
    QextJson.h


target.path = $$[QT_INSTALL_PREFIX]/lib
headers.files = $$PUBLIC_HEADERS
headers.path = $$[QT_INSTALL_PREFIX]/include/QtExtjson

INSTALLS += target headers
