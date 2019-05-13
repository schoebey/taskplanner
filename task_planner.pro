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
    task_manager \
    plugins \
    application/application.pro
