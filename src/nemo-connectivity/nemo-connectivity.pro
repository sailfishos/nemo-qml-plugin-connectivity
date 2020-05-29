TEMPLATE = lib
TARGET = nemoconnectivity

CONFIG += \
        c++11 \
        hide_symbols \
        link_pkgconfig \
        create_pc \
        create_prl \
        no_install_prl

QT = dbus network

INCLUDEPATH += ..

DEFINES += NEMO_BUILD_CONNECTIVITY_LIBRARY

PKGCONFIG += connman-qt5 \
    qofonoext \
    qofono-qt5

SOURCES += \
        connectionhelper.cpp \
        mobiledataconnection.cpp


PUBLIC_HEADERS += \
        connectionhelper.h \
        mobiledataconnection.h \
        global.h

HEADERS += $$PUBLIC_HEADERS \
    mobiledataconnection_p.h \

public_headers.files = $$PUBLIC_HEADERS
public_headers.path = $$PREFIX/include/nemo-connectivity

target.path = $$[QT_INSTALL_LIBS]

QMAKE_PKGCONFIG_NAME = nemoconnectivity
QMAKE_PKGCONFIG_DESCRIPTION = Nemo library for Connectivity
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$public_headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
QMAKE_PKGCONFIG_VERSION = $$VERSION

INSTALLS += \
        public_headers \
        target
