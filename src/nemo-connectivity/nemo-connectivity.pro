TEMPLATE = lib
TARGET = nemoconnectivity

CONFIG += \
        c++11 \
        hide_symbols \
        link_pkgconfig \
        create_pc \
        create_prl \
        no_install_prl

QT -= gui
QT += dbus

INCLUDEPATH += ..

DEFINES += NEMO_BUILD_CONNECTIVITY_LIBRARY

PKGCONFIG += connman-qt5 \
    qofonoext \
    qofono-qt5

SOURCES += mobiledataconnection.cpp

PUBLIC_HEADERS += \
        mobiledataconnection.h \
        global.h

HEADERS += $$PUBLIC_HEADERS \
    mobiledataconnection_p.h \

public_headers.files = $$PUBLIC_HEADERS
public_headers.path = /usr/include/nemo-connectivity

target.path = /usr/lib

QMAKE_PKGCONFIG_NAME = nemoconnectivity
QMAKE_PKGCONFIG_DESCRIPTION = Nemo library for Connectivity
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = /usr/include
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_VERSION = $$VERSION

INSTALLS += \
        public_headers \
        target
