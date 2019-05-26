CONFIG += c++11
QT += core gui
TEMPLATE = lib

win32 {
  DESTDIR = $$PWD
}
else:macx {
  DESTDIR = $$PWD/../../../taskplanner.app/Contents/MacOS/plugins/serializers/markdown
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


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../ -ltask_manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../ -ltask_manager
else:unix: LIBS += -L$$PWD/../../../../ -ltask_manager

INCLUDEPATH += $$PWD/../../../task_manager \
               $$PWD/../../../task_manager/serializers
DEPENDPATH += $$PWD/../../../task_manager
