cmake_minimum_required(VERSION 3.5)

project(markdown-serializer VERSION 0.8.0)

include(common)

include(common)

add_library(${PROJECT_NAME} SHARED
    markdownserializer.cpp
    markdownserializer.h
    markdownserializerplugin.cpp
    markdownserializerplugin.h
)

target_link_libraries(${PROJECT_NAME} PUBLIC libtaskmanager)

include(plugin-install-target)
include(plugin-output-dir)
