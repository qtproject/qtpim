load(qt_module)

QT += versit versit-private contacts

TARGET = qtversit_backuphandler
target.path += $$[QT_INSTALL_PLUGINS]/versit
INSTALLS += target

load(qt_plugin)

DESTDIR = $$QT.versit.plugins/versit

HEADERS += backupvcardhandler.h
SOURCES += backupvcardhandler.cpp
