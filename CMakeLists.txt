cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if(UNIX)
  SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")
endif()

set(APPLICATION_NAME taskplanner)
set(ORGANIZATION_NAME ancor)

project(taskplanner VERSION 0.8.0)

add_subdirectory(application)
add_subdirectory(libtaskmanager)
add_subdirectory(plugins)
add_subdirectory(unittests)
