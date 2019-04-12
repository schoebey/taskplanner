CONFIG += c++11
QT += core gui network
TEMPLATE = subdirs

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}


DISTFILES += \
    notes.txt \
    todos.md

SUBDIRS += \
    task_manager \
    plugins \
    application/application.pro

task_manager.path = $$PWD/task_manager
plugins.path = $$PWD/plugins
plugins.depends = task_manager
application.path = $$PWD/application
application.depends = task_manager
INSTALLS += task_manager
