CONFIG += c++11
QT += core

macx {
  INCLUDEPATH += /usr/local/include
  LIBS += -L/usr/local/lib
}

LIBS += -L$$OUT_PWD/../lib/

INCLUDEPATH += $$PWD/.. \
               $$PWD/../task_manager \
               $$PWD/../task_manager/serializers \
               $$PWD/../task_manager/reports

DEPENDPATH += $$PWD/../task_manager

LIBS *= -ltask_manager \
        -lgtest

SOURCES += \
    test_main.cpp \
    test_taskmanager.cpp \
    test_task.cpp \
    test_group.cpp \
    test_conversion.cpp \
    test_properties.cpp

