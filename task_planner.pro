CONFIG += c++11
QT += core gui
TEMPLATE = subdirs

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}


DISTFILES += \
    notes.txt \
    todos.md

SUBDIRS += \
    plugins \
    application/application.pro \
    unittests \
    libtaskmanager \
    libtaskmanager/libtaskmanager_static.pro \

libtaskmanager.path = $$PWD/libtaskmanager
plugins.path = $$PWD/plugins
plugins.depends = libtaskmanager
application.path = $$PWD/application
application.depends = libtaskmanager
unittests.path = $$PWD/unittests
unittests.depends = libtaskmanager
INSTALLS += libtaskmanager
