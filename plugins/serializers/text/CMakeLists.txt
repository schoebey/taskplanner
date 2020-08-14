cmake_minimum_required(VERSION 3.5)

project(text-serializer VERSION ${APP_VERSION})

include(common)

add_library(${PROJECT_NAME} SHARED
    textserializer.cpp
    textserializer.h
    textserializerplugin.cpp
    textserializerplugin.h
)

target_link_libraries(${PROJECT_NAME} PUBLIC libtaskmanager)

include(plugin-install-target)
include(plugin-output-dir)
