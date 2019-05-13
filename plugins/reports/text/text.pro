CONFIG += c++11
QT += core gui
TEMPLATE = lib
TARGET = $$PWD/../../../taskplanner.app/Contents/MacOS/plugins/reports/text

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}

HEADERS += \
    textreport.h \
    textreportplugin.h

SOURCES += \
    textreport.cpp \
    textreportplugin.cpp


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../../task_manager/release/ -ltask_manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../../task_manager/debug/ -ltask_manager
else:unix: LIBS += -L$$OUT_PWD/../../../task_manager/ -ltask_manager

INCLUDEPATH += $$PWD/../../../task_manager \
               $$PWD/../../../task_manager/serializers \
               $$PWD/../../../task_manager/reports
DEPENDPATH += $$PWD/../../../task_manager
