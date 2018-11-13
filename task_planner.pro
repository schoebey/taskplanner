CONFIG += c++11
QT += core gui

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}


include(task_manager/task_manager.pri)
include(gui/gui.pri)

SOURCES += \
    main.cpp
