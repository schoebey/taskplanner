cmake_minimum_required(VERSION 3.5)

project(libtaskmanager VERSION ${APP_VERSION})

include(common)

add_library(${PROJECT_NAME} SHARED
    constraint.cpp
    constraint.h
    constraint_grammar.h
    constraintfactory.cpp
    constraintfactory.h
    conversion.cpp
    conversion.h
    conversion_fancy.cpp
    conversion_fancy.h
    group.cpp
    group.h
    groupinterface.h
    interfaces/applicationinterface.h
    id_generator.h
    id_types.h
    libtaskmanager.h
    manager.cpp
    manager.h
    parameter.h
    parametrizable.cpp
    parametrizable.h
    parametrizableinterface.h
    patterns/factory/factory.cpp
    patterns/factory/factory.h
    patterns/factory/register.h
    patterns/factory/registrar.h
    plugininterface.h
    priority.h
    property.cpp
    property.h
    propertyproviderinterface.h
    reports/report.cpp
    reports/report.h
    reports/reportenums.h
    reports/reportfactory.h
    reports/reportinterface.h
    reports/reportregister.h
    serializableinterface.h
    serializablemanager.cpp
    serializablemanager.h
    serializers/serializationenums.h
    serializers/serializerfactory.h
    serializers/serializerinterface.h
    serializers/serializerregister.h
    tag.cpp
    tag.h
    task.cpp
    task.h
    taskinterface.h
    timefragment.h
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        .
        ..
        patterns
        reports
        serializers
)

find_package(Qt5 COMPONENTS Core Widgets)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt5::Widgets
        Qt5::Core
)

include(lib-install-target)
include(lib-output-dir)
