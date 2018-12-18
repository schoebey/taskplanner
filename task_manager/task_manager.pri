include(serializers/serializers.pri)

SOURCES += \
    $$PWD/group.cpp \
    $$PWD/manager.cpp \
    $$PWD/task.cpp \
    $$PWD/serializablemanager.cpp \
    $$PWD/property.cpp \
    $$PWD/constraint.cpp \
    $$PWD/conversion.cpp \
    $$PWD/constraintfactory.cpp \
    $$PWD/parametrizable.cpp

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
    $$PWD/container_utils.h \
    $$PWD/property.h \
    $$PWD/constraint.h \
    $$PWD/conversion.h \
    $$PWD/constraintfactory.h \
    $$PWD/parametrizable.h \
    $$PWD/parameter.h \
    $$PWD/parametrizableinterface.h
