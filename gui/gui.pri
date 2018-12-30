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
    $$PWD/taskcreationdialog.cpp

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
    $$PWD/taskcreationdialog.h

FORMS += \
    gui/groupwidget.ui \
    gui/mainwindow.ui \
    gui/taskwidget.ui

RESOURCES += \
    $$PWD/resources/resource.qrc
