INCLUDEPATH += $$PWD

SOURCES += \
    gui/editablelabel.cpp \
    gui/groupwidget.cpp \
    gui/mainwindow.cpp \
    gui/taskwidget.cpp \
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
    $$PWD/commands/changegrouppropertycommand.cpp

HEADERS += \
    gui/editablelabel.h \
    gui/groupwidget.h \
    gui/mainwindow.h \
    gui/taskwidget.h \
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
    $$PWD/commands/changegrouppropertycommand.h

FORMS += \
    gui/groupwidget.ui \
    gui/mainwindow.ui \
    gui/taskwidget.ui \
    $$PWD/linkwidget.ui

RESOURCES += \
    $$PWD/resources/resource.qrc
