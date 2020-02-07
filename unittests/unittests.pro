CONFIG += c++11
QT += core

macx {
  INCLUDEPATH += /usr/local/include
  LIBS += -L/usr/local/lib
}


INCLUDEPATH += $$PWD/.. \
               $$PWD/../libtaskmanager \
               $$PWD/../libtaskmanager/serializers \
               $$PWD/../libtaskmanager/reports

DEPENDPATH += $$PWD/../libtaskmanager

LIBS += -L../lib/
LIBS *= -ltaskmanager_static \
        -lgtest

SOURCES += \
    test_main.cpp \
    test_taskmanager.cpp \
    test_task.cpp \
    test_group.cpp \
    test_conversion.cpp \
    test_properties.cpp

