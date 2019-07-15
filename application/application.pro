CONFIG += c++11
QT += core gui

APP_NAME = taskplanner
ORGANIZATION_NAME = ancor
DEFINES += APP_NAME=\\\"$$APP_NAME\\\" \
           ORGANIZATION_NAME=\\\"$$ORGANIZATION_NAME\\\"

TARGET = $$APP_NAME
macx {
  DESTDIR = ../
}
else {
  DESTDIR = ../bin
}

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
}

include(../git_version.pri)

SOURCES += \
    $$PWD/editablelabel.cpp \
    $$PWD/groupwidget.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/taskwidget.cpp \
    $$PWD/main.cpp \
    $$PWD/style.cpp \
    $$PWD/highlightanimation.cpp \
    $$PWD/taskwidgetoverlay.cpp \
    $$PWD/overlaywidget.cpp \
    $$PWD/taskcreationdialog.cpp \
    $$PWD/flowlayout.cpp \
    $$PWD/linkwidget.cpp \
    $$PWD/commands/movetaskcommand.cpp \
    $$PWD/commands/addtaskcommand.cpp \
    $$PWD/widgetmanager.cpp \
    $$PWD/commands/changetaskpropertycommand.cpp \
    $$PWD/commands/changegrouppropertycommand.cpp \
    $$PWD/commands/deletetaskcommand.cpp \
    $$PWD/mousehandlingframe.cpp \
    $$PWD/aboutdialog.cpp \
    hibernationdetector.cpp \
    tasklistwidget.cpp \
    toolbarinfodisplay.cpp \
    widgetresizer.cpp \
    windowtitlemenubar.cpp \
    decoratedlabel.cpp

HEADERS += \
    $$PWD/editablelabel.h \
    $$PWD/groupwidget.h \
    $$PWD/mainwindow.h \
    $$PWD/taskwidget.h \
    $$PWD/style.h \
    $$PWD/highlightanimation.h \
    $$PWD/taskwidgetoverlay.h \
    $$PWD/highlightmethod.h \
    $$PWD/overlaywidget.h \
    $$PWD/taskcreationdialog.h \
    $$PWD/flowlayout.h \
    $$PWD/linkwidget.h \
    $$PWD/commands/movetaskcommand.h \
    $$PWD/commands/addtaskcommand.h \
    $$PWD/widgetmanager.h \
    $$PWD/commands/changetaskpropertycommand.h \
    $$PWD/commands/changegrouppropertycommand.h \
    $$PWD/commands/deletetaskcommand.h \
    $$PWD/mousehandlingframe.h \
    $$PWD/aboutdialog.h \
    hibernationdetector.h \
    itaskcontainerwidget.h \
    tasklistwidget.h \
    toolbarinfodisplay.h \
    widgetresizer.h \
    windowtitlemenubar.h \
    decoratedlabel.h

FORMS += \
    $$PWD/groupwidget.ui \
    $$PWD/mainwindow.ui \
    $$PWD/taskwidget.ui \
    $$PWD/linkwidget.ui \
    $$PWD/aboutdialog.ui

RESOURCES += \
    $$PWD/resources/resource.qrc

macx: LIBS += -L$$OUT_PWD/../taskplanner.app/Contents/MacOS/
else: LIBS += -L$$OUT_PWD/../bin/

LIBS *= -ltask_manager

unix {
    !macx: QMAKE_LFLAGS +=  -Wl,-rpath=\'\$$ORIGIN/\'
}

INCLUDEPATH += $$PWD \
               $$PWD/../task_manager \
               $$PWD/../task_manager/serializers \
               $$PWD/../task_manager/reports \
DEPENDPATH += $$PWD/../task_manager
