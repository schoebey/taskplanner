CONFIG += c++11
QT += core gui
TEMPLATE = lib
TARGET = text

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


macx: LIBS += -L$$OUT_PWD/../../../taskplanner.app/Contents/MacOS/
else: LIBS += -L$$OUT_PWD/../../../bin/

LIBS *= -ltask_manager

INCLUDEPATH += $$PWD/../../../task_manager \
               $$PWD/../../../task_manager/serializers \
               $$PWD/../../../task_manager/reports
DEPENDPATH += $$PWD/../../../task_manager
