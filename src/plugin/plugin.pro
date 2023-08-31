TARGET = nemoconnectivity
PLUGIN_IMPORT_PATH = Nemo/Connectivity

TEMPLATE = lib
CONFIG += qt plugin hide_symbols
QT = qml dbus
LIBS += -lconnman-qt$${QT_MAJOR_VERSION} -L../nemo-connectivity -lnemoconnectivity
target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

INCLUDEPATH += $$PWD/.. $$PWD/../nemo-connectivity

qmldir.files += qmldir plugins.qmltypes
qmldir.path +=  $$target.path
INSTALLS += qmldir

qmltypes.commands = qmlplugindump -nonrelocatable Nemo.Connectivity 1.0 > $$PWD/plugins.qmltypes
QMAKE_EXTRA_TARGETS += qmltypes

SOURCES += plugin.cpp
