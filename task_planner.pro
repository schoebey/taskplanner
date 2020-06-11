CONFIG += c++11
QT += core gui
TEMPLATE = subdirs

# -fno-limit-debug-info

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}

SUBDIRS += \
    plugins \
    application/application.pro \
    unittests \
    libtaskmanager \
    libtaskmanager_static \

libtaskmanager.path = $$PWD/libtaskmanager
libtaskmanager_static.path = $$PWD/libtaskmanager_static
plugins.path = $$PWD/plugins
plugins.depends = libtaskmanager
application.path = $$PWD/application
application.depends = libtaskmanager
unittests.path = $$PWD/unittests
unittests.depends = libtaskmanager_static
INSTALLS += libtaskmanager
