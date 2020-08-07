cmake_minimum_required(VERSION 3.5)

project(unittests VERSION 0.2.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
    gtest_helpers.h
    test_conversion.cpp
    test_group.cpp
    test_main.cpp
    test_properties.cpp
    test_task.cpp
    test_taskmanager.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC libtaskmanager)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ..
        ../libtaskmanager/
        ../libtaskmanager/patterns
        ../libtaskmanager/reports
        ../libtaskmanager/serializers
)

find_package(GTest)
find_package(Qt5 COMPONENTS Core Widgets)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Widgets
        Qt5::Core
        GTest::gtest
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
