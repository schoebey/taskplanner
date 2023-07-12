cmake_minimum_required(VERSION 3.5)

set(APPLICATION_NAME taskplanner)
set(APP_VERSION 0.8.4)
set(ORGANIZATION_NAME ancor)

project(taskplanner VERSION ${APP_VERSION})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(auto-ccache)
include(installdirs)
include(rpath)

add_subdirectory(application)
add_subdirectory(libtaskmanager)
add_subdirectory(plugins)
add_subdirectory(unittests)
