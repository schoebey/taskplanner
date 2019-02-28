CONFIG += c++11
QT += core gui

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}

include(git_version.pri)
include(task_manager/task_manager.pri)
include(gui/gui.pri)

DISTFILES += \
    notes.txt \
    todos.md
