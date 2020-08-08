cmake_minimum_required(VERSION 3.5)

project(taskplanner VERSION 0.8.0)

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
    widgetmanager.cpp
    widgetmanager.h
    widgetresizer.cpp
    widgetresizer.h
    windowtitlemenubar.cpp
    windowtitlemenubar.h
)

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


include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# TODO: Remove when version header is generated
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ORGANIZATION_NAME="ancor"
    APP_NAME="${PROJECT_NAME}"
    GIT_VERSION="1231efe"
    GIT_TIMESTAMP="2020-08-03"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

