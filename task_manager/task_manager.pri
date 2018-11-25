include(serializers/serializers.pri)

SOURCES += \
    $$PWD/group.cpp \
    $$PWD/manager.cpp \
    $$PWD/task.cpp \
    $$PWD/serializablemanager.cpp

HEADERS += \
    $$PWD/group.h \
    $$PWD/id_types.h \
    $$PWD/manager.h \
    $$PWD/task.h \
    $$PWD/timefragment.h \
    $$PWD/priority.h \
    $$PWD/id_generator.h \
    $$PWD/serializableinterface.h \
    $$PWD/taskinterface.h \
    $$PWD/groupinterface.h \
    $$PWD/serializablemanager.h \
    $$PWD/container_utils.h
