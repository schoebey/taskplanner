cmake_minimum_required(VERSION 3.5)

project(unittests VERSION ${APP_VERSION})

find_package(GTest)

if(NOT GTest_FOUND)
    # No system/vcpkg GTest available (e.g. windows-latest CI runners).
    # Fetch and build GoogleTest from source using the same compiler
    # already selected for this build (works uniformly with the MinGW
    # toolchain used for the Windows build, unlike vcpkg's default MSVC
    # triplet, which would need extra triplet configuration for MinGW).
    cmake_minimum_required(VERSION 3.14)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    set(GTest_FOUND TRUE)
endif()

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

add_test(NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME} --gtest_output=xml:${CMAKE_BINARY_DIR}/test-results/${PROJECT_NAME}.xml
)

if(WIN32)
    # libtaskmanager's DLL is built into build/libtaskmanager (its own
    # subdirectory binary dir), not build/bin where unittests.exe lives,
    # so the loader needs an extra PATH entry to find it at test time.
    set_tests_properties(${PROJECT_NAME} PROPERTIES
        ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}/libtaskmanager;$ENV{PATH}"
    )
endif()

endif(GTest_FOUND)
