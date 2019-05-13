CONFIG += c++11
QT += core gui
TEMPLATE = lib

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}

include(../git_version.pri)

INCLUDEPATH *= \
    serializers

HEADERS += \
    constraint.h \
    constraintfactory.h \
    container_utils.h \
    conversion.h \
    group.h \
    groupinterface.h \
    id_generator.h \
    id_types.h \
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
    reports/reportregister.h

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

