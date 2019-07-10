CONFIG += c++11
QT += core gui
TEMPLATE = lib
TARGET = text

win32 {
  DESTDIR = $$PWD/../../../bin/plugins/reports/
}
else:macx {
  DESTDIR = $$PWD/../../../taskplanner.app/Contents/MacOS/plugins/reports
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


win32: LIBS += -L$$PWD/../../../bin/ -ltask_manager
else:unix: LIBS += -L$$PWD/../../../taskplanner.app/Contents/MacOS/ -ltask_manager

INCLUDEPATH += $$PWD/../../../task_manager \
               $$PWD/../../../task_manager/serializers \
               $$PWD/../../../task_manager/reports
DEPENDPATH += $$PWD/../../../task_manager
