cmake_minimum_required(VERSION 3.5)

project(text-serializer VERSION 0.2.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME} SHARED
    textserializer.cpp
    textserializer.h
    textserializerplugin.cpp
    textserializerplugin.h
)

target_link_libraries(${PROJECT_NAME} PUBLIC libtaskmanager)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ../../../libtaskmanager
        ../../../libtaskmanager/reports
        ../../../libtaskmanager/serializers
)

find_package(Qt5 COMPONENTS Core Widgets)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Widgets
        Qt5::Core
)

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/plugins/serializers
)
