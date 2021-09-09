cmake_minimum_required(VERSION 3.5)

set(APPLICATION_NAME taskplanner)
set(APP_VERSION 0.8.0)
set(ORGANIZATION_NAME ancor)
#set(CMAKE_C_COMPILER_WORKS)
#set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(taskplanner VERSION ${APP_VERSION})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(auto-ccache)
include(installdirs)
include(rpath)

add_subdirectory(application)
add_subdirectory(libtaskmanager)
add_subdirectory(plugins)
add_subdirectory(unittests)
