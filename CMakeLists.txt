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

set(RUNTIME_INSTALL_DIR bin
    CACHE PATH "Install dir for executables and dlls")
set(ARCHIVE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib
    CACHE PATH "Install dir for static libraries")
set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib
    CACHE PATH "Install dir for shared libraries")
set(PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/${APPLICATION_NAME}
    CACHE PATH "Install dir for plugins")
mark_as_advanced(RUNTIME_INSTALL_DIR ARCHIVE_INSTALL_DIR  LIBRARY_INSTALL_DIR PLUGIN_INSTALL_DIR)

project(taskplanner VERSION ${APP_VERSION})

add_subdirectory(application)
add_subdirectory(libtaskmanager)
add_subdirectory(plugins)
add_subdirectory(unittests)
