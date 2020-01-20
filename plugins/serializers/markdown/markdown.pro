CONFIG += c++11
QT += core gui
TEMPLATE = lib
TARGET = markdown

macx {
  DESTDIR = ../../../taskplanner.app/Contents/MacOS/plugins/serializers
}
else {
  DESTDIR = ../../../bin/plugins/serializers/
}

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}

HEADERS += \
    markdownserializer.h \
    markdownserializerplugin.h

SOURCES += \
    markdownserializer.cpp \
    markdownserializerplugin.cpp

LIBS += -L$$OUT_PWD/../../../lib/

LIBS *= -ltask_manager

INCLUDEPATH += $$PWD/../../../task_manager \
               $$PWD/../../../task_manager/serializers
DEPENDPATH += $$PWD/../../../task_manager
