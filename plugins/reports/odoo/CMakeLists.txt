cmake_minimum_required(VERSION 3.5)

project(odoo-report VERSION 0.8.0)

include(common)

add_library(${PROJECT_NAME} SHARED
    odooreport.cpp
    odooreport.h
    plugin.cpp
    plugin.h
)

target_link_libraries(${PROJECT_NAME} PUBLIC libtaskmanager)

include(plugin-install-target)
include(plugin-output-dir)
