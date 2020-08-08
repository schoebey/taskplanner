cmake_minimum_required(VERSION 3.5)

project(text-report VERSION 0.8.0)

include(common)

add_library(${PROJECT_NAME} SHARED
    textreport.cpp
    textreport.h
    textreportplugin.cpp
    textreportplugin.h
)

target_link_libraries(${PROJECT_NAME} PUBLIC libtaskmanager)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ../../../libtaskmanager
        ../../../libtaskmanager/reports
        ../../../libtaskmanager/serializers
)

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/${APPLICATION_NAME}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${APPLICATION_NAME}
)
