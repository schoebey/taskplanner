CONFIG += c++11
QT += core gui
TEMPLATE = lib
TARGET = text

include(../../../variables.pri)

macx {
  DESTDIR = ../../../taskplanner.app/Contents/MacOS/plugins/reports
}
else {
  DESTDIR = ../../../bin/plugins/reports/
}

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}

HEADERS += \
    textreport.h \
    textreportplugin.h

SOURCES += \
    textreport.cpp \
    textreportplugin.cpp

LIBS += -L$$LIB_DIR

LIBS *= -ltaskmanager

INCLUDEPATH += $$PWD/../../../libtaskmanager \
               $$PWD/../../../libtaskmanager/serializers \
               $$PWD/../../../libtaskmanager/reports
DEPENDPATH += $$PWD/../../../libtaskmanager
