CONFIG += c++11
QT += core gui
TEMPLATE = lib
TARGET = text

win32 {
  DESTDIR = $$PWD/../../../bin/plugins/serializers/
}
else:macx {
  DESTDIR = $$PWD/../../../taskplanner.app/Contents/MacOS/plugins/serializers
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


win32: LIBS += -L$$PWD/../../../bin/ -ltask_manager
else:unix: LIBS += -L$$PWD/../../../taskplanner.app/Contents/MacOS/ -ltask_manager

INCLUDEPATH += $$PWD/../../../task_manager \
               $$PWD/../../../task_manager/serializers
DEPENDPATH += $$PWD/../../../task_manager
