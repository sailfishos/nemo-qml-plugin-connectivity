TARGET = nemoconnectivity
PLUGIN_IMPORT_PATH = Nemo/Connectivity

TEMPLATE = lib
CONFIG += qt plugin hide_symbols
QT += qml network dbus
LIBS += -lconnman-qt5 -L../nemo-connectivity -lnemoconnectivity
target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

INCLUDEPATH += $$PWD/.. $$PWD/../nemo-connectivity

qmldir.files += $$_PRO_FILE_PWD_/qmldir
qmldir.path +=  $$target.path
INSTALLS += qmldir

SOURCES += plugin.cpp connectionhelper.cpp
HEADERS += connectionhelper_p.h
