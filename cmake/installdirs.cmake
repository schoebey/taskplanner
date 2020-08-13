set(RUNTIME_INSTALL_DIR bin
    CACHE PATH "Install dir for executables and dlls")

set(ARCHIVE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib
    CACHE PATH "Install dir for static libraries")

set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib
    CACHE PATH "Install dir for shared libraries")

set(PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/${APPLICATION_NAME}
    CACHE PATH "Install dir for plugins")

mark_as_advanced(RUNTIME_INSTALL_DIR ARCHIVE_INSTALL_DIR  LIBRARY_INSTALL_DIR PLUGIN_INSTALL_DIR)

