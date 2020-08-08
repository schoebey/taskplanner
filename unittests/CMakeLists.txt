cmake_minimum_required(VERSION 3.5)

project(unittests VERSION ${APP_VERSION})

find_package(GTest)

if(GTest_FOUND)

include(common)

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

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        GTest::gtest
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

endif(GTest_FOUND)
