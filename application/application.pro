CONFIG += c++11
QT += core gui
TARGET = taskplanner
DESTDIR = $$PWD/../

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
    $$PWD/commands/addsubtaskcommand.cpp \
    $$PWD/mousehandlingframe.cpp \
    $$PWD/aboutdialog.cpp

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
    $$PWD/commands/addsubtaskcommand.h \
    $$PWD/mousehandlingframe.h \
    $$PWD/aboutdialog.h

FORMS += \
    $$PWD/groupwidget.ui \
    $$PWD/mainwindow.ui \
    $$PWD/taskwidget.ui \
    $$PWD/linkwidget.ui \
    $$PWD/aboutdialog.ui

RESOURCES += \
    $$PWD/resources/resource.qrc


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../ -ltask_manager
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../ -ltask_manager
else:unix: LIBS += -L$$PWD/../ -ltask_manager

INCLUDEPATH += $$PWD \
               $$PWD/../task_manager \
               $$PWD/../task_manager/serializers \
               $$PWD/../task_manager/reports \
DEPENDPATH += $$PWD/../task_manager
