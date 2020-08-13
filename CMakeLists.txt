cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if(UNIX)
  SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")
endif()

set(APPLICATION_NAME taskplanner)
set(APP_VERSION 0.8.0)
set(ORGANIZATION_NAME ancor)

include(auto-ccache)
include(installdirs)

project(taskplanner VERSION ${APP_VERSION})

add_subdirectory(application)
add_subdirectory(libtaskmanager)
add_subdirectory(plugins)
add_subdirectory(unittests)
