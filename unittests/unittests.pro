CONFIG += c++11
QT += core

macx {
  INCLUDEPATH += /usr/local/include
  LIBS += -L/usr/local/lib
}

LIBS += -L$$OUT_PWD/../lib/

INCLUDEPATH += $$PWD/.. \
               $$PWD/../libtaskmanager \
               $$PWD/../libtaskmanager/serializers \
               $$PWD/../libtaskmanager/reports

DEPENDPATH += $$PWD/../libtaskmanager

LIBS *= -ltaskmanager \
        -lgtest

SOURCES += \
    test_main.cpp \
    test_taskmanager.cpp \
    test_task.cpp \
    test_group.cpp \
    test_conversion.cpp \
    test_properties.cpp

