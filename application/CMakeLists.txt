cmake_minimum_required(VERSION 3.5)

project(taskplanner VERSION ${APP_VERSION})

include(common)

add_executable(${PROJECT_NAME}
    aboutdialog.cpp
    aboutdialog.h
    aboutdialog.ui
    blendableimagewidget.cpp
    blendableimagewidget.h
    commands/addtaskcommand.cpp
    commands/addtaskcommand.h
    commands/changegrouppropertycommand.cpp
    commands/changegrouppropertycommand.h
    commands/changetaskpropertycommand.cpp
    commands/changetaskpropertycommand.h
    commands/deletetaskcommand.cpp
    commands/deletetaskcommand.h
    commands/movetaskcommand.cpp
    commands/movetaskcommand.h
    decoratedlabel.cpp
    decoratedlabel.h
    editablelabel.cpp
    editablelabel.h
    floatingwidget.cpp
    floatingwidget.h
    flowlayout.cpp
    flowlayout.h
    groupwidget.cpp
    groupwidget.h
    groupwidget.ui
    hibernationdetector.cpp
    hibernationdetector.h
    highlightanimation.cpp
    highlightanimation.h
    highlightmethod.h
    itaskcontainerwidget.h
    linkwidget.cpp
    linkwidget.h
    linkwidget.ui
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    mousehandlingframe.cpp
    mousehandlingframe.h
    overlaywidget.cpp
    overlaywidget.h
    resources/resource.qrc
    search/matchinfo.h
    search/searchcontroller.cpp
    search/searchcontroller.h
    search/searchframe.cpp
    search/searchframe.h
    search/searchframe.ui
    search/searchoptions.h
    style.cpp
    style.h
    styleExtension.h
    taskcreationdialog.cpp
    taskcreationdialog.h
    tasklistwidget.cpp
    tasklistwidget.h
    taskwidget.cpp
    taskwidget.h
    taskwidget.ui
    taskwidgetoverlay.cpp
    taskwidgetoverlay.h
    toolbarinfodisplay.cpp
    toolbarinfodisplay.h
    version.h.in
    widgetmanager.cpp
    widgetmanager.h
    widgetresizer.cpp
    widgetresizer.h
    windowtitlemenubar.cpp
    windowtitlemenubar.h
)

include(version)

target_link_libraries(${PROJECT_NAME} PUBLIC libtaskmanager)

find_package(Qt5 COMPONENTS Qml Gui)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::Qml
        Qt5::Gui
)

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC pthread)
endif()

include(app-install-target)
include(app-output-dir)

add_subdirectory(resources)

if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD COMMAND
        ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/../Frameworks/"
        $<TARGET_FILE:${PROJECT_NAME}>)
endif()
