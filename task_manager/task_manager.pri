include(serializers/serializers.pri)

SOURCES += \
    $$PWD/group.cpp \
    $$PWD/manager.cpp \
    $$PWD/task.cpp

HEADERS += \
    $$PWD/group.h \
    $$PWD/id_types.h \
    $$PWD/manager.h \
    $$PWD/task.h \
    $$PWD/timefragment.h \
    $$PWD/priority.h \
    $$PWD/id_generator.h
