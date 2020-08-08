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

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${APPLICATION_NAME}
)
