include(text/text.pri)
include(markdown/markdown.pri)

INCLUDEPATH += $$PWD/../ \

SOURCES += \
    $$PWD/serializerregistrar.cpp \
    $$PWD/serializerfactory.cpp \
    $$PWD/serializer.cpp

HEADERS += \
    $$PWD/serializerregistrar.h \
    $$PWD/serializerinterface.h \
    $$PWD/register.h \
    $$PWD/serializerfactory.h \
    $$PWD/serializerfactoryprivate.h \
    $$PWD/serializer.h \
    $$PWD/serializationenums.h \
    $$PWD/serializerinfo.h
