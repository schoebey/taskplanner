INCLUDEPATH += $$PWD

SOURCES += \
    gui/editablelabel.cpp \
    gui/groupwidget.cpp \
    gui/mainwindow.cpp \
    gui/taskwidget.cpp \
    $$PWD/main.cpp \
    $$PWD/style.cpp

HEADERS += \
    gui/editablelabel.h \
    gui/groupwidget.h \
    gui/mainwindow.h \
    gui/taskwidget.h \
    $$PWD/style.h

FORMS += \
    gui/groupwidget.ui \
    gui/mainwindow.ui \
    gui/taskwidget.ui

RESOURCES += \
    $$PWD/resources/resource.qrc
