include(text/text.pri)
include(markdown/markdown.pri)

INCLUDEPATH += $$PWD/../ \

SOURCES += \
    $$PWD/serializerregistrar.cpp \
    $$PWD/serializerfactory.cpp

HEADERS += \
    $$PWD/serializerregistrar.h \
    $$PWD/serializerinterface.h \
    $$PWD/register.h \
    $$PWD/serializerfactory.h \
    $$PWD/serializerfactoryprivate.h \
    $$PWD/serializationenums.h \
    $$PWD/serializerinfo.h
