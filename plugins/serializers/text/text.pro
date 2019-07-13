CONFIG += c++11
QT += core gui
TEMPLATE = lib
TARGET = text

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
    textserializer.h \
    textserializerplugin.h

SOURCES += \
    textserializer.cpp \
    textserializerplugin.cpp


macx: LIBS += -L$$OUT_PWD/../../../taskplanner.app/Contents/MacOS/
else: LIBS += -L$$OUT_PWD/../../../bin/

LIBS *= -ltask_manager

INCLUDEPATH += $$PWD/../../../task_manager \
               $$PWD/../../../task_manager/serializers
DEPENDPATH += $$PWD/../../../task_manager
