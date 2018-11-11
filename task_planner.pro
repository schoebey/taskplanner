QT += widgets
CONFIG += c++11


# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    group.cpp \
    manager.cpp \
    task.cpp \
    mainwindow.cpp \
    taskwidget.cpp \
    groupwidget.cpp \
    editablelabel.cpp

RESOURCES +=

HEADERS += \
    group.h \
    manager.h \
    task.h \
    mainwindow.h \
    taskwidget.h \
    groupwidget.h \
    id_types.h \
    editablelabel.h

FORMS += \
    mainwindow.ui \
    taskwidget.ui \
    groupwidget.ui
