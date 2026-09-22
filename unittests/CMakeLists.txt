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
    test_textreport.cpp
    ${CMAKE_SOURCE_DIR}/plugins/reports/text/textreport.cpp
    ${CMAKE_SOURCE_DIR}/plugins/reports/text/textreport.h
)

target_link_libraries(${PROJECT_NAME} PUBLIC libtaskmanager)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        GTest::gtest
)

include(app-output-dir)

enable_testing()
add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME} --gtest_output=xml:${CMAKE_BINARY_DIR}/test-results/${PROJECT_NAME}.xml
)

endif(GTest_FOUND)
