cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if(UNIX)
  SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")
endif()

set(APPLICATION_NAME taskplanner)
set(APP_VERSION 0.8.0)
set(ORGANIZATION_NAME ancor)

# Auto. CCACHE
if(UNIX)
    find_program(CCACHE_FOUND ccache)

    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif(CCACHE_FOUND)
endif()

project(taskplanner VERSION ${APP_VERSION})

add_subdirectory(application)
add_subdirectory(libtaskmanager)
add_subdirectory(plugins)
add_subdirectory(unittests)
