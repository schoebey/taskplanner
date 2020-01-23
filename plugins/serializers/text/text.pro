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

LIBS += -L$$OUT_PWD/../../../lib/

LIBS *= -ltaskmanager

INCLUDEPATH += $$PWD/../../../libtaskmanager \
               $$PWD/../../../libtaskmanager/serializers
DEPENDPATH += $$PWD/../../../libtaskmanager
