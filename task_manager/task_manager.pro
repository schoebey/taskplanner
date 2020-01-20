CONFIG += c++11 skip_target_version_ext
QT += core gui
TEMPLATE = lib
CONFIG += static

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}

DESTDIR = ../lib

include(../git_version.pri)
include(patterns/patterns.pri)

DEFINES += BUILD_TASKMANAGER_LIB

INCLUDEPATH *= \
    serializers

HEADERS += \
    constraint.h \
    constraintfactory.h \
    conversion.h \
    group.h \
    groupinterface.h \
    id_generator.h \
    id_types.h \
    libtaskmanager.h \
    manager.h \
    parameter.h \
    parametrizable.h \
    parametrizableinterface.h \
    plugininterface.h \
    priority.h \
    property.h \
    propertyproviderinterface.h \
    task.h \
    taskinterface.h \
    timefragment.h \
    serializableinterface.h \
    serializablemanager.h \
    serializers/serializationenums.h \
    serializers/serializerfactory.h \
    serializers/serializerinterface.h \
    reports/report.h \
    reports/reportenums.h \
    reports/reportfactory.h \
    reports/reportinterface.h \
    reports/reportregister.h \
    libtaskmanager.h

SOURCES += \
    constraint.cpp \
    constraintfactory.cpp \
    conversion.cpp \
    group.cpp \
    manager.cpp \
    parametrizable.cpp \
    property.cpp \
    task.cpp \
    serializablemanager.cpp \
    reports/report.cpp

